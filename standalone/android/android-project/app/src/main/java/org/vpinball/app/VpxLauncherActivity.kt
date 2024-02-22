package org.vpinball.app

import android.Manifest.permission.READ_EXTERNAL_STORAGE
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.app.Activity
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager.PERMISSION_GRANTED
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Environment
import android.os.ParcelFileDescriptor
import android.provider.DocumentsContract
import android.provider.Settings
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Info
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ElevatedButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.snapshots.SnapshotStateList
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.content.edit
import androidx.core.net.toUri
import androidx.documentfile.provider.DocumentFile
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import org.vpinball.app.ui.theme.AndroidprojectTheme
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException
import java.io.InputStream

class VpxLauncherActivity : ComponentActivity() {
    val tablesList = mutableStateListOf("")
    val isConfigured = mutableStateOf(false)
    val showErrorDialog = mutableStateOf(false)
    val showWorkingOverlay = mutableStateOf(false)
    val overlayMessage = mutableStateOf("")
    var vpxDir: Uri? = null
    var vpxDirFd: ParcelFileDescriptor? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Retrieve the previously selected VPX directory from preferences
        vpxDir = getPreferences(Context.MODE_PRIVATE).let {
            it.getString(VPX_DIR_KEY, null)?.toUri()
        }

        // The launcher will not display the initial warning page if there is a configured
        // VPX directory and the user has granted storage permissions
        isConfigured.value = (vpxDir != null) && checkStoragePermissions()

        // Read all tables from the VPX directory
        if (isConfigured.value) openVpxDirectory()

        setContent {
            AndroidprojectTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    if (showErrorDialog.value) {
                        AlertDialog(
                            icon = { Icon(Icons.Default.Info, contentDescription = "Alert") },
                            title = { Text(text = stringResource(R.string.dialog_title)) },
                            text = { Text(text = stringResource(R.string.dialog_text)) },
                            onDismissRequest = { showErrorDialog.value = false },
                            confirmButton = { },
                            dismissButton = {
                                TextButton(onClick = { showErrorDialog.value = false }) {
                                    Text("Ok")
                                }
                            }
                        )
                    }

                    if (isConfigured.value)
                        PinballTables(tablesList)
                    else
                        InitialMessage()

                    if (showWorkingOverlay.value) {
                        WorkingOverlay(overlayMessage)
                    }
                }
            }
        }
    }

    private fun requestStoragePermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            try {
                val uri = Uri.fromParts("package", this.packageName, null)
                val intent = Intent()
                    .setAction(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION)
                    .setData(uri)
                storageActivityResultLauncher.launch(intent)
            } catch (e: Exception) {
                val intent = Intent().setAction(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION)
                storageActivityResultLauncher.launch(intent)
            }
        } else {
            ActivityCompat.requestPermissions(this,
                arrayOf(WRITE_EXTERNAL_STORAGE, READ_EXTERNAL_STORAGE), PERMISSION_REQUEST_CODE)
        }
    }

    private val storageActivityResultLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()) {
        showErrorDialog.value = !Environment.isExternalStorageManager()
        isConfigured.value = !showErrorDialog.value
    }

    private fun checkStoragePermissions(): Boolean {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Environment.isExternalStorageManager()
        } else {
            val write = ContextCompat.checkSelfPermission(this, WRITE_EXTERNAL_STORAGE)
            val read = ContextCompat.checkSelfPermission(this, READ_EXTERNAL_STORAGE)
            read == PERMISSION_GRANTED && write == PERMISSION_GRANTED
        }
    }

    // For Android versions below 11
    override fun onRequestPermissionsResult(requestCode: Int,
                                            permissions: Array<String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            PERMISSION_REQUEST_CODE -> {
                showErrorDialog.value = !(grantResults.isNotEmpty() && grantResults[0] == PERMISSION_GRANTED)
                isConfigured.value = !showErrorDialog.value
                return
            }
        }
    }

    fun openVpxDirectory(updateAssets: Boolean = false) {
        if (!checkStoragePermissions()) requestStoragePermissions()

        val contentResolver = applicationContext.contentResolver
        var doc: DocumentFile?
        try {
            doc = DocumentFile.fromTreeUri(this, vpxDir!!)
            vpxDirFd = contentResolver.openFileDescriptor(doc!!.uri, "r")

            // There must be a tables directory under the selected VPX dir
            // Make sure it exists now even though it will be eventually
            // created by copying it from assets
            var tablesDir = doc.findFile("tables")
            if (tablesDir == null) {
                tablesDir = doc.createDirectory("tables")
            }

            if (updateAssets) {
                // Show an overlay while copying asset files
                // On physical devices this can take quile a while
                Log.d(TAG, "Copying assets")
                overlayMessage.value = getString(R.string.copying_assets_message)
                showWorkingOverlay.value = true

                // Copy files using a background thread
                lifecycleScope.launch(Dispatchers.IO) {
                    copyAssets("", doc)

                    Log.d(TAG, "Done copying assets to the selected VPX dir")
                    overlayMessage.value = ""
                    showWorkingOverlay.value = false

                    updateTables(tablesDir!!)
                }
            }

            // Tell Compose to display the list of tables
            isConfigured.value = true

            updateTables(tablesDir!!)
        } catch (e: Exception) {
            // Request access from the user either if there is no VPX directory selected
            // or we cannot access it anymore
            val suggestedVpxDir = File(Environment.getExternalStorageDirectory(), "vpx").toUri()
            askUserForVpxDirectory(suggestedVpxDir)
        }
    }

    // Add all VPX tables to the UI
    private fun updateTables(tablesDir: DocumentFile) {
        tablesList.clear()
        tablesDir!!.listFiles()
            .filter {
                it.name?.endsWith(".vpx", ignoreCase = true) == true
            }.forEach {
                val tableName = it.name!!.dropLast(4)
                tablesList.add(tableName)
            }
    }

    @Throws(IOException::class)
    private fun copyAssets(srcDir: String, dstDir: DocumentFile) {
        val files = assets.list(srcDir)
        for (filename in files!!) {
            var srcPath = ""
            if (!srcDir.isEmpty()) {
                srcPath = "$srcDir/"
            }
            srcPath += filename

            var dstFile = dstDir.findFile(filename)
            if (dstFile != null && dstFile.isFile) continue

            try {
                val inputStream = assets.open(srcPath)
                dstFile = dstDir.createFile("application/octet-stream", filename.replace(' ', '_'))!!
                Log.v(TAG, "Copying $srcPath to '${dstFile.name}'")
                copyFile(inputStream, dstFile)
            } catch (e: FileNotFoundException) {
                // Create the directory if it doesn't exist
                // The logic goes inside an existing directory in case
                // there files missing in the existing directory
                copyAssets(srcPath, dstFile ?: dstDir.createDirectory(filename)!!)
            } catch (e: IOException) {
                Log.v(TAG, "Unable to copy $srcPath", e)
            }
        }
    }

    @Throws(IOException::class)
    private fun copyFile(source: InputStream, dstFile: DocumentFile) {
        val buf = ByteArray(1024)
        applicationContext.contentResolver.openOutputStream(dstFile.uri)?.use {
            var len: Int
            while (source.read(buf).also { len = it } > 0) {
                it.write(buf, 0, len)
            }
            source.close()
        }
    }

    fun askUserForVpxDirectory(pickerInitialUri: Uri) {
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT_TREE).apply {
            putExtra(DocumentsContract.EXTRA_INITIAL_URI, pickerInitialUri)
        }

        startActivityForResult(intent, VPX_DIR_CODE)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, resultData: Intent?) {
        super.onActivityResult(requestCode, resultCode, resultData)
        if (requestCode == VPX_DIR_CODE && resultCode == Activity.RESULT_OK) {
            resultData?.data?.also { uri ->
                val contentResolver = applicationContext.contentResolver

                // Tell Android we will need this permission forever
                val takeFlags = Intent.FLAG_GRANT_READ_URI_PERMISSION or
                        Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                contentResolver.takePersistableUriPermission(uri, takeFlags)

                // Store the uri in the app's preferences file
                // This way we don't need to ask the user again
                getPreferences(Context.MODE_PRIVATE).edit {
                    this.putString(VPX_DIR_KEY, uri.toString())
                }

                vpxDir = uri
                openVpxDirectory(true)
            }
        }
    }

    companion object {
        private const val TAG = "VPinballActivity"
        private const val VPX_DIR_KEY = "VPXMainDir"
        private const val VPX_DIR_CODE = 1000
        private const val PERMISSION_REQUEST_CODE = 1001
    }
}

@Composable
fun InitialMessage() {
    val activity = LocalContext.current as VpxLauncherActivity

    Column(modifier = Modifier.padding(horizontal = 14.dp)) {

        Column(modifier = Modifier.weight(1f).fillMaxWidth(),
            horizontalAlignment = Alignment.CenterHorizontally) {
            Spacer(modifier = Modifier.height(48.dp))
            Text(stringResource(R.string.select_dir_msg))
            Box(contentAlignment = Alignment.Center, modifier = Modifier.fillMaxSize()) {
                Button(
                    onClick = {
                        // Try to open the VPX directory
                        activity.openVpxDirectory(true)
                    }
                ) {
                    Text(stringResource(R.string.button_select_dir))
                }
            }
        }

        Text(text = stringResource(R.string.permissions_msg),
            fontStyle = FontStyle.Italic,
            modifier = Modifier.padding(vertical = 24.dp))
    }
}

// Rudimentary presentation of all tables on a list.
// It works but it's ugly. Hopefully to be replaced with a nice looking UI.
@Composable
fun PinballTables(tablesList: SnapshotStateList<String>) {
    val tables = remember { tablesList }
    val activity = LocalContext.current as VpxLauncherActivity

    Column {
        Text(stringResource(R.string.main_title),
            fontSize = 24.sp, textAlign = TextAlign.Center, modifier = Modifier.fillMaxWidth())

        Spacer(modifier = Modifier.height(24.dp))

        Column(verticalArrangement = Arrangement.spacedBy(8.dp),
            modifier = Modifier
                .verticalScroll(rememberScrollState())
                .weight(1f)
                .fillMaxWidth()
                .padding(horizontal = 8.dp)
        ) {
            tables.forEach { table ->
                Text(text = table,
                    modifier = Modifier.fillMaxWidth().clickable {
                        val intent = Intent(activity, VPinballActivity::class.java).apply {
                            putExtra("table", "$table.vpx")
                            putExtra("dirFd", activity.vpxDirFd!!.detachFd())
                        }
                        activity.startActivity(intent)
                    })

                HorizontalDivider()
            }
        }

        Spacer(modifier = Modifier.height(16.dp))

        Row(horizontalArrangement = Arrangement.SpaceEvenly,
            modifier = Modifier
                .padding(8.dp)
                .fillMaxWidth()) {
            ElevatedButton(onClick = { activity.openVpxDirectory(false) }) {
                Text(stringResource(R.string.button_refresh))
            }

            ElevatedButton(onClick = {
                val suggestedVpxDir = File(Environment.getExternalStorageDirectory(), "vpx").toUri()
                activity.askUserForVpxDirectory(suggestedVpxDir)
            }) {
                Text(stringResource(R.string.button_new_dir))
            }
        }

    }
}


@Composable
fun WorkingOverlay(message: MutableState<String>) {
    Box(Modifier
        .fillMaxSize()
        .background(Color.Gray.copy(alpha = 0.5f))
        .pointerInput(Unit) {}
    ) {
        Column(
            modifier = Modifier.matchParentSize().padding(horizontal = 16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            CircularProgressIndicator()
            Spacer(modifier = Modifier.height(24.dp))
            Text(message.value)
        }
    }
}