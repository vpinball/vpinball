package org.vpinball.app

import android.content.Context
import android.net.Uri
import androidx.browser.customtabs.CustomTabsIntent
import java.io.File
import org.vpinball.app.jni.VPinballDisplayText

object BuildInfo {
    const val PROVENANCE = "Official app from the vpinball project"

    val version: String
        get() = VPinballManager.getVersionString()
}

enum class Link(val url: String) {
    DOCS("https://github.com/vpinball/vpinball/blob/master/docs/Mobile.md"),
    TROUBLESHOOTING("https://github.com/vpinball/vpinball/blob/master/docs/Mobile.md#troubleshooting"),
    DISCORD("https://discord.com/channels/652274650524418078/1323445406524248090"),
    LICENSES("https://github.com/vpinball/vpinball/blob/master/LICENSE"),
    ZEDMDOS("https://github.com/PPUC/zedmdos"),
    VPINBALL("https://github.com/vpinball/vpinball"),
    PINMAME("https://github.com/vpinball/pinmame"),
    LIBALTSOUND("https://github.com/vpinball/libaltsound"),
    LIBDMDUTIL("https://github.com/vpinball/libdmdutil"),
    LIBZEDMD("https://github.com/PPUC/libzedmd"),
    LIBSERUM("https://github.com/PPUC/libserum"),
    LIBDOF("https://github.com/vpinball/libdof"),
    LIBVNI("https://github.com/PPUC/libvni"),
    LIBWINEVBS("https://github.com/vpinball/libwinevbs"),
    THIRDPARTY("https://github.com/vpinball/vpinball/blob/master/third-party/README.md");

    fun open(context: Context) {
        val intent = CustomTabsIntent.Builder().build()
        intent.launchUrl(context, Uri.parse(url))
    }
}

enum class Credit(val displayName: String, val authors: String? = null, val link: Link? = null) {
    VPINBALL(
        "Visual Pinball",
        "toxieainc, vbousquet, fuzzelhjb, jsm174, francisdb, c-f-h, bcd, cupidsf, djrobx, gitfool, brandrew2, mjrgh, koadic76, superhac, shagendo, Nicals, CraftedCart, horseyhorsey, kara2010, snail_gary, cwick, Le-Syl21, Matthias Buecher, baxelrod-bdai, claytgreene, YellowLabrador, mkalkbrenner, JockeJarre, markmon, ScaryG, WildCoder, freezy, nkissebe, omigeot, dekay, Pyrrvs, Wylted1, Chickenzilla, RandyDavis2000, latsao, ntleverenz, WizardsHat, garybrowndev, mcragun, hughfitzgerald, dynajoe, herrMirto, evilwraith, nicolaspr56, colas-sebastien, Herschel, rockfordroeNG, Yuki, KutsuyaYuki, teamsuperpanda, joni999, surtarso, andremichi, jermatic1, LeHaine, kaicherry, Billiam, droscoe, CapitaineSheridan, ravarcade, RockfordRoe, cschmidtpxc, manofwar32, poiuyterry",
        Link.VPINBALL,
    ),
    PINMAME(
        "PinMAME",
        "toxieainc, volkenborn, Steve Ellenoff, bcd, Tom Haukap, wpcmame, Matthias Buecher, vbousquet, jsm174, gerwout, mkalkbrenner, droscoe, djrobx, tomlogic, Thomas Behrens, bontango, mjrgh, Oliver Kaegi, whaslbeck, syllebra, JockeJarre, evilwraith, gitfool, francisdb, gstellenberg, Randall Perlow, Le-Syl21, freezy, Netsplits, jayadelson, bhitney, gnulnulf, Samasaur1, Disservin, mattwalsh, Mark Sunnucks, kara2010, coryaltheide, Herschel, noflip95, diego-link-eggy, Pavel Sereda, uid68989",
        Link.PINMAME,
    ),
    LIBALTSOUND("libaltsound", "jsm174, toxieainc, Le-Syl21, gitfool, francisdb", Link.LIBALTSOUND),
    LIBDMDUTIL("libdmdutil", "mkalkbrenner, jsm174, toxieainc, djrobx, PastorL69, Al Linke, bartdesign, francisdb", Link.LIBDMDUTIL),
    LIBZEDMD("libzedmd", "mkalkbrenner, jsm174, zesinger, PastorL69, Cpasjuste, bartdesign", Link.LIBZEDMD),
    LIBSERUM("libserum", "zesinger, mkalkbrenner, pinballpower, jsm174, vbousquet, toxieainc", Link.LIBSERUM),
    LIBDOF("libdof", "jsm174, dynajoe, christiancoleman, dejaloomer, patsoffice, superhac, dekay", Link.LIBDOF),
    LIBVNI("libvni", "jsm174, mkalkbrenner, djrobx", Link.LIBVNI),
    LIBWINEVBS("libwinevbs", "jsm174, francisdb, gitfool", Link.LIBWINEVBS),
    ARTWORK("Artwork", "smillard316 (Table placeholder), adam.co (App icon enhancements)"),
    OTHER("Other third party libraries", link = Link.THIRDPARTY),
}

enum class TableGridSize(val value: Int) {
    SMALL(0),
    MEDIUM(1),
    LARGE(2);

    fun columns(wide: Boolean, compactHeight: Boolean): Int =
        when (this) {
            SMALL -> if (compactHeight) 10 else if (wide) 6 else 4
            MEDIUM -> if (compactHeight) 8 else if (wide) 4 else 3
            LARGE -> if (compactHeight) 6 else if (wide) 3 else 2
        }

    companion object {
        fun fromInt(value: Int): TableGridSize = entries.firstOrNull { it.value == value } ?: MEDIUM
    }
}

enum class AppAppearance(val value: Int, override val text: String) : VPinballDisplayText {
    DARK(0, "Dark"),
    LIGHT(1, "Light"),
    SYSTEM(2, "System");

    companion object {
        fun fromInt(value: Int): AppAppearance = entries.firstOrNull { it.value == value } ?: DARK
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
