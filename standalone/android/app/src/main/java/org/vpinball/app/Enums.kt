package org.vpinball.app

import android.content.Context
import android.net.Uri
import androidx.browser.customtabs.CustomTabsIntent
import java.io.File

enum class Link(val url: String) {
    DOCS("https://github.com/vpinball/vpinball/blob/master/standalone/docs/ios/README.md"),
    TROUBLESHOOTING("https://github.com/vpinball/vpinball/blob/master/standalone/docs/ios/README.md#troubleshooting"),
    DISCORD("https://discord.com/channels/652274650524418078/1323445406524248090"),
    LICENSES("https://github.com/vpinball/vpinball/blob/master/LICENSE"),
    ZEDMDOS("https://github.com/PPUC/zedmdos"),
    VPINBALL("https://github.com/vpinball/vpinball"),
    PINMAME("https://github.com/vpinball/pinmame"),
    LIBALTSOUND("https://github.com/vpinball/libaltsound"),
    LIBDMDUTIL("https://github.com/vpinball/libdmdutil"),
    LIBZEDMD("https://github.com/PPUC/libzedmd"),
    LIBSERUM("https://github.com/PPUC/libserum"),
    LIBDOF("https://github.com/jsm174/libdof"),
    LIBVNI("https://github.com/PPUC/libvni"),
    THIRDPARTY("https://github.com/vpinball/vpinball/blob/master/third-party/README.md");

    fun open(context: Context) {
        val intent = CustomTabsIntent.Builder().build()
        intent.launchUrl(context, Uri.parse(url))
    }
}

enum class Credit(val displayName: String, val authors: String? = null, val link: Link? = null) {
    VPINBALL(
        "Visual Pinball",
        "toxieainc, vbousquet, fuzzelhjb, jsm174, c-f-h, francisdb, bcd, cupidsf, djrobx, brandrew2, mjrgh, koadic76, shagendo, Nicals, horseyhorsey, CraftedCart, superhac, snail_gary, Matthias Buecher, Le-Syl21, baxelrod-bdai, YellowLabrador, claytgreene, markmon, JockeJarre, WildCoder, ScaryG, nkissebe, mkalkbrenner, freezy, Wylted1, WizardsHat, RandyDavis2000, ntleverenz, latsao, Chickenzilla, Yuki, teamsuperpanda, surtarso, RockfordRoe, ravarcade, poiuyterry, omigeot, manofwar32, LeHaine, KutsuyaYuki, kaicherry, joni999, jmarzka, droscoe, cschmidtpxc, CapitaineSheridan, Billiam, andremichi, evilwraith",
        Link.VPINBALL,
    ),
    PINMAME(
        "PinMAME",
        "toxieainc, volkenborn, Steve Ellenoff, bcd, Tom, Haukap, wpcmame, Matthias Buecher, jsm174, vbousquet, mkalkbrenner, droscoe, djrobx, Thomas Behrens, bontango, tomlogic, mjrgh, Oliver, Kaegi, syllebra, JockeJarre, Randall, Perlow, gstellenberg, Netsplits, gnulnulf, Sunnucks, mattwalsh, Mark, freezy, uid68989, Sereda, Pavel, noflip95, No, francisdb, diego-link-eggy",
        Link.PINMAME,
    ),
    LIBALTSOUND("libaltsound", "droscoe, jsm174, toxieainc, francisdb", Link.LIBALTSOUND),
    LIBDMDUTIL("libdmdutil", "mkalkbrenner, jsm174, toxieainc, francisdb, bartdesign, freezy", Link.LIBDMDUTIL),
    LIBZEDMD("libzedmd", "mkalkbrenner, jsm174, zesinger, Cpasjuste, bartdesign", Link.LIBZEDMD),
    LIBSERUM("libserum", "zesinger, mkalkbrenner, pinballpower, jsm174, vbousquet, toxieainc", Link.LIBSERUM),
    LIBDOF("libdof", "jsm174, dekay", Link.LIBDOF),
    LIBVNI("libvni", "mkalkbrenner, freezy, jsm174", Link.LIBVNI),
    ARTWORK("Artwork", "smillard316 (Table placeholder), adam.co (App icon enhancements)"),
    OTHER("Other third party libraries", link = Link.THIRDPARTY),
}

enum class TableViewMode(val value: Int) {
    GRID(0),
    LIST(1);

    companion object {
        fun fromInt(value: Int): TableViewMode = entries.firstOrNull { it.value == value } ?: GRID
    }
}

enum class TableGridSize(val value: Int) {
    SMALL(0),
    MEDIUM(1),
    LARGE(2);

    companion object {
        fun fromInt(value: Int): TableGridSize = entries.firstOrNull { it.value == value } ?: MEDIUM
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

enum class CodeLanguage(val extension: String, val monacoType: String) {
    INI("ini", monacoType = "ini"),
    LOG("log", monacoType = "plaintext"),
    VBSCRIPT("vbs", monacoType = "vb"),
    TXT("txt", monacoType = "plaintext");

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

enum class TableImageState {
    NO_IMAGE,
    LOADING_IMAGE,
    IMAGE_LOADED,
}
