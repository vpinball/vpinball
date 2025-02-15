package org.vpinball.app

import android.content.Context
import android.net.Uri
import androidx.browser.customtabs.CustomTabsIntent
import java.io.File

enum class Link(val url: String) {
    DOCS("https://github.com/vpinball/vpinball/blob/master/standalone/docs/ios/README.md"),
    TROUBLESHOOTING("https://github.com/vpinball/vpinball/blob/master/standalone/docs/ios/README.md#troubleshooting"),
    DISCORD("https://discord.com/channels/652274650524418078/1076655472846831667"),
    LICENSES("https://github.com/vpinball/vpinball/blob/master/LICENSE"),
    ZEDMDOS("https://github.com/PPUC/zedmdos"),
    VPINBALL("https://github.com/vpinball/vpinball"),
    PINMAME("https://github.com/vpinball/pinmame"),
    LIBALTSOUND("https://github.com/vpinball/libaltsound"),
    LIBDMDUTIL("https://github.com/vpinball/libdmdutil"),
    LIBZEDMD("https://github.com/ppuc/libzedmd"),
    LIBSERUM("https://github.com/zesinger/libserum"),
    LIBDOF("https://github.com/jsm174/libdof"),
    THIRDPARTY("https://github.com/vpinball/vpinball/blob/master/third-party/README.md");

    fun open(context: Context) {
        val intent = CustomTabsIntent.Builder().build()
        intent.launchUrl(context, Uri.parse(url))
    }
}

enum class Credit(val displayName: String, val authors: String? = null, val link: Link? = null) {
    VPINBALL(
        "Visual Pinball",
        "toxieainc, vbousquet, fuzzelhjb, jsm174, c-f-h, bcd, cupidsf, djrobx, brandrew2, francisdb, koadic76, " +
            "shagendo, horseyhorsey, CraftedCart, snail_gary, Matthias, Buecher, Nicals, YellowLabrador, markmon, nkissebe, " +
            "mkalkbrenner, freezy, WildCoder, randydavis99, ntleverenz, latsao, Wylted1, WizardsHat, ScaryG, Chickenzilla, " +
            "ravarcade, poiuyterry, omigeot, mjrgh, manofwar32, kaicherry, droscoe, cschmidtpxc, RockfordRoe, JockeJarre, " +
            "CapitaineSheridan, Billiam, LeHaine, claytgreene",
        Link.VPINBALL,
    ),
    PINMAME(
        "PinMAME",
        "volkenborn, toxieainc, Steve Ellenoff, bcd, Tom Haukap, wpcmame, Matthias Buecher, jsm174, mkalkbrenner, " +
            "droscoe, vbousquet, djrobx, Thomas Behrens, bontango, mjrgh, Oliver Kaegi, syllebra, gstellenberg, Randall, " +
            "Perlow, JockeJarre, tomlogic, Netsplits, gnulnulf, freezy, Sunnucks, Mark, uid68989, francisdb, diego-link-eggy, " +
            "Sereda, Pavel, tomlogic",
        Link.PINMAME,
    ),
    LIBALTSOUND("libaltsound", "droscoe, jsm174, toxieainc, francisdb", Link.LIBALTSOUND),
    LIBDMDUTIL("libdmdutil", "mkalkbrenner, jsm174, toxieainc, bartdesign, francisdb, freezy", Link.LIBDMDUTIL),
    LIBZEDMD("libzedmd", "mkalkbrenner, jsm174, zesinger, bartdesign", Link.LIBZEDMD),
    LIBSERUM("libserum", "zesinger, mkalkbrenner, pinballpower, jsm174, toxieainc", Link.LIBSERUM),
    LIBDOF("libdof", "zesinger, mkalkbrenner, jsm174", Link.LIBDOF),
    ARTWORK(
        "Artwork",
        "smillard316 (Table placeholder), adam.co (App icon enhancements), Juan Pablo Bravo (The Noun Project 17955), " +
            "Zach Bogart (The Noun Project 3169564), Templarian (SVG Repo 369724)",
    ),
    OTHER("Other third party libraries", link = Link.THIRDPARTY),
}

enum class TableListMode(val value: Int) {
    TWO_COLUMN(0),
    THREE_COLUMN(1),
    LIST(2);

    companion object {
        fun fromInt(value: Int): TableListMode {
            return entries.firstOrNull { it.value == value } ?: TWO_COLUMN
        }
    }
}

enum class TableListSortOrder(val value: Int) {
    A_Z(0),
    Z_A(1);

    companion object {
        fun fromInt(value: Int): TableListSortOrder {
            return entries.firstOrNull { it.value == value } ?: A_Z
        }
    }
}

enum class CodeLanguage(val extension: String, val prismType: String) {
    INI("ini", prismType = "ini"),
    LOG("log", prismType = "log"),
    VBSCRIPT("vbs", prismType = "visual-basic"),
    TXT("txt", prismType = "text");

    companion object {
        fun fromExtension(extension: String): CodeLanguage {
            return entries.firstOrNull { it.extension.equals(extension, ignoreCase = true) } ?: TXT
        }

        fun fromFile(file: File): CodeLanguage {
            val fileExtension = file.extension.takeIf { it.isNotEmpty() }?.lowercase()
            return fileExtension?.let { fromExtension(it) } ?: TXT
        }
    }
}

enum class ArtworkState {
    NO_IMAGE,
    LOADING_IMAGE,
    IMAGE_LOADED,
}
