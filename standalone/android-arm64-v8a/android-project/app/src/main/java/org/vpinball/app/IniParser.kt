package org.vpinball.app

import android.content.Context
import androidx.documentfile.provider.DocumentFile
import java.io.BufferedReader
import java.io.InputStreamReader

fun loadIniSection(context: Context, vpxIni: DocumentFile?, section: String): Map<String, Any> {
    val result = HashMap<String, Any>()

    if (vpxIni == null) return result

    context.contentResolver.openInputStream(vpxIni.uri)?.use {
        val reader = BufferedReader(InputStreamReader(it))
        var line: String?
        var readValues = false

        while (reader.readLine().also { line = it } != null) {
            line = line!!.trim()
            if (line!!.isEmpty()) continue

            val firstChar = line!![0]
            if (firstChar == '[') {
                // End of the section, no need to read the rest of the file
                if (readValues) break

                readValues = line!!.substring(1, line!!.length - 1) == section
            } else if (firstChar in ";#/") {
                // Ignore comments
                continue
            } else if (readValues) {
                val equals = line!!.indexOf('=')
                if (equals > 0) {
                    val key = line!!.substring(0, equals).trim()
                    val value = line!!.substring(equals + 1).trim()
                    result[key] = value
                }
            }
        }
    }

    return result
}

