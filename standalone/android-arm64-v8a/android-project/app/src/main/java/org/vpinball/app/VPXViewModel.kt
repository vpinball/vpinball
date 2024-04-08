package org.vpinball.app

import android.Manifest
import android.app.Application
import android.content.Context
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.os.Environment
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.core.content.ContextCompat
import androidx.core.content.edit
import androidx.core.net.toUri
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.documentfile.provider.DocumentFile
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import kotlinx.coroutines.MainScope
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.launch
import java.lang.Exception

class VPXViewModelFactory(private val application: Application, private val dataStore: DataStore<Preferences>): ViewModelProvider.NewInstanceFactory() {
    override fun <T:ViewModel> create(modelClass: Class<T>): T = VPXViewModel(application, dataStore) as T
}

class VPXViewModel(application: Application, dataStore: DataStore<Preferences>): AndroidViewModel(application) {
    var vpxDir: Uri? = null
    val isConfigured = mutableStateOf(false)
    val tablesList = mutableStateListOf("")
    var webServerState = dataStore.data
        .map { it[booleanPreferencesKey("webserver")] ?: false }

    init {
        System.loadLibrary("vpinball")

        // Retrieve the previously selected VPX directory from preferences
        vpxDir = application.getSharedPreferences(PREFS_FILE, Context.MODE_PRIVATE).let {
            it.getString(VPX_DIR_KEY, null)?.toUri()
        }

        // The launcher will not display the initial warning page if there is a configured
        // VPX directory and the user has granted storage permissions
        isConfigured.value = (vpxDir != null) && checkStoragePermissions()

        // Start/Stop the webserver according to the settings value
        MainScope().launch {
            webServerState.collect {
                // Read the configuration values from the INI before starting the webserver
                if (it) {
                    val vpxDir = DocumentFile.fromTreeUri(application, vpxDir!!)
                    val iniDocFile = vpxDir?.findFile("VPinballX.ini")

                    val standaloneIni = loadIniSection(application, iniDocFile, "Standalone")
                    val addr = (standaloneIni["WebServerAddr"] as String?).takeIf { it.isNullOrEmpty() }?.let {
                        "0.0.0.0"
                    }
                    val port = (standaloneIni["WebServerPort"] as String?).let {
                        try {
                            it!!.toInt()
                        } catch (e: Exception) {
                            2112
                        }
                    }
                    val isDebug = (standaloneIni["WebServerDebug"] as String?).let {
                        try {
                            it!!.toBoolean()
                        } catch (e: Exception) {
                            false
                        }
                    }
                    val contentResolver = application.contentResolver
                    contentResolver.openFileDescriptor(vpxDir!!.uri, "r").use {
                        initwebserver(addr!!, port, isDebug, it!!.fd)
                    }
                }

                webserver(it)
            }
        }
    }

    private external fun initwebserver(addr: String, port: Int, debug: Boolean, fd: Int)
    private external fun webserver(state: Boolean)

    fun checkStoragePermissions(): Boolean {
        val context = getApplication<Application>().applicationContext
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Environment.isExternalStorageManager()
        } else {
            val write = ContextCompat.checkSelfPermission(context,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            )
            val read = ContextCompat.checkSelfPermission(context,
                Manifest.permission.READ_EXTERNAL_STORAGE
            )
            read == PackageManager.PERMISSION_GRANTED && write == PackageManager.PERMISSION_GRANTED
        }
    }

    fun updateVPXDir(uri: Uri) {
        val context = getApplication<Application>().applicationContext
        context.getSharedPreferences(PREFS_FILE, Context.MODE_PRIVATE).edit {
            this.putString(VPX_DIR_KEY, uri.toString())
        }
        vpxDir = uri
    }

    companion object {
        private const val TAG = "VPXViewModel"
        private const val VPX_DIR_KEY = "VPXMainDir"
        private const val PREFS_FILE = "vpx"
    }
}