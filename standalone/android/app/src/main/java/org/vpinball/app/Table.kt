package org.vpinball.app

import java.io.File
import kotlinx.serialization.Serializable

@Serializable
data class Table(val uuid: String, val name: String, val path: String, val image: String, val createdAt: Long, val modifiedAt: Long) {
    val fileName: String
        get() = File(path).name

    val fullPath: String
        get() {
            val tablesPath = VPinballManager.getTablesPath()
            return if (SAFFileSystem.isUsingSAF()) {
                "$tablesPath/$path"
            } else {
                File(tablesPath, path).absolutePath
            }
        }

    val baseURL: File
        get() =
            if (SAFFileSystem.isUsingSAF()) {
                File("")
            } else {
                File(fullPath).parentFile ?: File("")
            }

    val basePath: String
        get() =
            if (SAFFileSystem.isUsingSAF()) {
                fullPath.substringBeforeLast('/')
            } else {
                baseURL.absolutePath
            }

    val scriptURL: File
        get() = File(fullPath.substringBeforeLast('.') + ".vbs")

    val scriptPath: String
        get() =
            if (SAFFileSystem.isUsingSAF()) {
                path.substringBeforeLast('.') + ".vbs"
            } else {
                scriptURL.absolutePath
            }

    val imagePath: String
        get() {
            val tablesPath = VPinballManager.getTablesPath()
            return if (SAFFileSystem.isUsingSAF()) {
                "$tablesPath/$image"
            } else {
                File(tablesPath, image).absolutePath
            }
        }
}
