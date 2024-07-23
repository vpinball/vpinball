package org.vpinball.app

import android.Manifest.permission.READ_EXTERNAL_STORAGE
import android.Manifest.permission.WRITE_EXTERNAL_STORAGE
import android.app.Activity
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
import androidx.activity.viewModels
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Info
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.MutableState
import androidx.compose.runtime.mutableStateOf
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.core.app.ActivityCompat
import androidx.core.net.toUri
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.preferencesDataStore
import androidx.documentfile.provider.DocumentFile
import androidx.lifecycle.lifecycleScope
import androidx.navigation.compose.rememberNavController
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import org.vpinball.app.ui.theme.AndroidprojectTheme
import java.io.File
import java.io.FileNotFoundException
import java.io.IOException
import java.io.InputStream

class VpxLauncherActivity : ComponentActivity() {
    val dataStore: DataStore<Preferences> by preferencesDataStore(name = "settings")
    val model by viewModels<VPXViewModel> { VPXViewModelFactory(application, dataStore) }

    val showErrorDialog = mutableStateOf(false)
    val showWorkingOverlay = mutableStateOf(false)
    val overlayMessage = mutableStateOf("")

    var vpxDocFile: DocumentFile? = null
    var vpxDirFd: ParcelFileDescriptor? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Read all tables from the VPX directory
        if (model.isConfigured.value) openVpxDirectory()

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

                    Navigator(controller = rememberNavController(), model = model)

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
        model.isConfigured.value = !showErrorDialog.value
    }

    // For Android versions below 11
    override fun onRequestPermissionsResult(requestCode: Int,
                                            permissions: Array<String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            PERMISSION_REQUEST_CODE -> {
                showErrorDialog.value = !(grantResults.isNotEmpty() && grantResults[0] == PERMISSION_GRANTED)
                model.isConfigured.value = !showErrorDialog.value
                return
            }
        }
    }

    fun openVpxDirectory(updateAssets: Boolean = false) {
        if (!model.checkStoragePermissions()) requestStoragePermissions()

        val contentResolver = applicationContext.contentResolver
        try {
            vpxDocFile = DocumentFile.fromTreeUri(this, model.vpxDir!!)
            vpxDirFd = contentResolver.openFileDescriptor(vpxDocFile!!.uri, "r")

            // There must be a tables directory under the selected VPX dir
            // Make sure it exists now even though it will be eventually
            // created by copying it from assets
            var tablesDir = vpxDocFile!!.findFile("tables")
            if (tablesDir == null) {
                tablesDir = vpxDocFile!!.createDirectory("tables")
            }

            if (updateAssets) {
                // Show an overlay while copying asset files
                // On physical devices this can take quile a while
                Log.d(TAG, "Copying assets")
                overlayMessage.value = getString(R.string.copying_assets_message)
                showWorkingOverlay.value = true

                // Copy files using a background thread
                lifecycleScope.launch(Dispatchers.IO) {
                    copyAssets("", vpxDocFile!!)

                    Log.d(TAG, "Done copying assets to the selected VPX dir")
                    overlayMessage.value = ""
                    showWorkingOverlay.value = false

                    updateTables(tablesDir!!)
                }
            }

            // Tell Compose to display the list of tables
            model.isConfigured.value = true

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
        model.tablesList.clear()
        tablesDir.listFiles()
            .filter {
                it.name?.endsWith(".vpx", ignoreCase = true) == true
            }.forEach {
                val tableName = it.name!!.dropLast(4)
                model.tablesList.add(tableName)
            }
    }

    @Throws(IOException::class)
    private fun copyAssets(srcDir: String, dstDir: DocumentFile) {
        val files = assets.list(srcDir)
        for (filename in files!!) {
            // Skip "hidden" files
            if (filename.startsWith(".")) {
                continue
            }

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
                model.updateVPXDir(uri)
                openVpxDirectory(true)
            }
        }
    }

    fun copyVPXLog(): File? {
        val copiedLog = File.createTempFile("vpx", ".log", getExternalCacheDir())
        copiedLog.deleteOnExit()

        vpxDocFile?.findFile("vpinball.log")?.let {
            applicationContext.contentResolver.openInputStream(it.uri)?.use {
                it.copyTo(copiedLog.outputStream())
            }
            return copiedLog
        }

        return null
    }

    companion object {
        private const val TAG = "VPinballActivity"
        private const val VPX_DIR_CODE = 1000
        private const val PERMISSION_REQUEST_CODE = 1001
    }
}


@Composable
fun WorkingOverlay(message: MutableState<String>) {
    Box(
        Modifier
            .fillMaxSize()
            .background(Color.Gray.copy(alpha = 0.5f))
            .pointerInput(Unit) {}
    ) {
        Column(
            modifier = Modifier
                .matchParentSize()
                .padding(horizontal = 16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            CircularProgressIndicator()
            Spacer(modifier = Modifier.height(24.dp))
            Text(message.value)
        }
    }
}