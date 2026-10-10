import SwiftUI

enum TableGridSize: Int, Hashable {
    case small = 0
    case medium = 1
    case large = 2

    func columns(regular: Bool, compactHeight: Bool) -> Int {
        switch self {
        case .small: return compactHeight ? 10 : regular ? 6 : 4
        case .medium: return compactHeight ? 8 : regular ? 4 : 3
        case .large: return compactHeight ? 6 : regular ? 3 : 2
        }
    }
}

enum AppAppearance: Int, CaseIterable, Identifiable {
    case dark = 0
    case light = 1
    case system = 2

    var id: Int { rawValue }

    var title: String {
        switch self {
        case .dark: return "Dark"
        case .light: return "Light"
        case .system: return "System"
        }
    }

    var style: UIUserInterfaceStyle {
        switch self {
        case .dark: return .dark
        case .light: return .light
        case .system: return .unspecified
        }
    }

    static var current: AppAppearance {
        AppAppearance(rawValue: VPinballManager.shared.loadValue(.standalone, "Appearance", AppAppearance.dark.rawValue)) ?? .dark
    }

    func apply() {
        for scene in UIApplication.shared.connectedScenes.compactMap({ $0 as? UIWindowScene }) {
            for window in scene.windows {
                window.overrideUserInterfaceStyle = style
            }
        }
    }
}

enum BuildInfo {
    static let provenance = "Official app from the vpinball project"

    static var version: String {
        String(cString: VPinballGetVersionStringFull())
    }
}

enum Link {
    case docs
    case troubleshooting
    case discord
    case licenses
    case zedmdos
    case vpinball
    case pinmame
    case libaltsound
    case libdmdutil
    case libzedmd
    case libserum
    case libdof
    case libvni
    case libwinevbs
    case thirdparty

    var url: URL {
        switch self {
        case .docs:
            return URL(string: "https://github.com/vpinball/vpinball/blob/master/docs/Mobile.md")!
        case .troubleshooting:
            return URL(string: "https://github.com/vpinball/vpinball/blob/master/docs/Mobile.md#troubleshooting")!
        case .discord:
            return URL(string: "https://discord.com/channels/652274650524418078/1323445406524248090")!
        case .licenses:
            return URL(string: "https://github.com/vpinball/vpinball/blob/master/LICENSE")!
        case .zedmdos:
            return URL(string: "https://github.com/PPUC/zedmdos")!
        case .vpinball:
            return URL(string: "https://github.com/vpinball/vpinball")!
        case .pinmame:
            return URL(string: "https://github.com/vpinball/pinmame")!
        case .libaltsound:
            return URL(string: "https://github.com/vpinball/libaltsound")!
        case .libdmdutil:
            return URL(string: "https://github.com/vpinball/libdmdutil")!
        case .libzedmd:
            return URL(string: "https://github.com/PPUC/libzedmd")!
        case .libserum:
            return URL(string: "https://github.com/PPUC/libserum")!
        case .libdof:
            return URL(string: "https://github.com/vpinball/libdof")!
        case .libvni:
            return URL(string: "https://github.com/PPUC/libvni")!
        case .libwinevbs:
            return URL(string: "https://github.com/vpinball/libwinevbs")!
        case .thirdparty:
            return URL(string: "https://github.com/vpinball/vpinball/blob/master/third-party/README.md")!
        }
    }

    func open() {
        if UIApplication.shared.canOpenURL(url) {
            UIApplication.shared.open(url)
        }
    }
}

enum Credit {
    case vpinball
    case pinmame
    case libaltsound
    case libdmdutil
    case libzedmd
    case libserum
    case libdof
    case libvni
    case libwinevbs
    case other
    case artwork

    static let all: [Credit] = [.vpinball,
                                .pinmame,
                                .libaltsound,
                                .libdmdutil,
                                .libzedmd,
                                .libserum,
                                .libdof,
                                .libvni,
                                .libwinevbs,
                                .other,
                                .artwork]

    var name: String {
        switch self {
        case .vpinball:
            return "Visual Pinball"
        case .pinmame:
            return "PinMAME"
        case .libaltsound:
            return "libaltsound"
        case .libdmdutil:
            return "libdmdutil"
        case .libzedmd:
            return "libzedmd"
        case .libserum:
            return "libserum"
        case .libdof:
            return "libdof"
        case .libvni:
            return "libvni"
        case .libwinevbs:
            return "libwinevbs"
        case .other:
            return "Other third party libraries"
        case .artwork:
            return "Artwork"
        }
    }

    var authors: String? {
        switch self {
        case .vpinball:
            return "toxieainc, vbousquet, fuzzelhjb, jsm174, francisdb, c-f-h, bcd, cupidsf, djrobx, gitfool, brandrew2, mjrgh, koadic76, superhac, shagendo, Nicals, CraftedCart, horseyhorsey, kara2010, snail_gary, cwick, Le-Syl21, Matthias Buecher, baxelrod-bdai, claytgreene, YellowLabrador, mkalkbrenner, JockeJarre, markmon, ScaryG, WildCoder, freezy, nkissebe, omigeot, dekay, Pyrrvs, Wylted1, Chickenzilla, RandyDavis2000, latsao, ntleverenz, WizardsHat, garybrowndev, mcragun, hughfitzgerald, dynajoe, herrMirto, evilwraith, nicolaspr56, colas-sebastien, Herschel, rockfordroeNG, Yuki, KutsuyaYuki, teamsuperpanda, joni999, surtarso, andremichi, jermatic1, LeHaine, kaicherry, Billiam, droscoe, CapitaineSheridan, ravarcade, RockfordRoe, cschmidtpxc, manofwar32, poiuyterry"
        case .pinmame:
            return "toxieainc, volkenborn, Steve Ellenoff, bcd, Tom Haukap, wpcmame, Matthias Buecher, vbousquet, jsm174, gerwout, mkalkbrenner, droscoe, djrobx, tomlogic, Thomas Behrens, bontango, mjrgh, Oliver Kaegi, whaslbeck, syllebra, JockeJarre, evilwraith, gitfool, francisdb, gstellenberg, Randall Perlow, Le-Syl21, freezy, Netsplits, jayadelson, bhitney, gnulnulf, Samasaur1, Disservin, mattwalsh, Mark Sunnucks, kara2010, coryaltheide, Herschel, noflip95, diego-link-eggy, Pavel Sereda, uid68989"
        case .libaltsound:
            return "jsm174, toxieainc, Le-Syl21, gitfool, francisdb"
        case .libdmdutil:
            return "mkalkbrenner, jsm174, toxieainc, djrobx, PastorL69, Al Linke, bartdesign, francisdb"
        case .libzedmd:
            return "mkalkbrenner, jsm174, zesinger, PastorL69, Cpasjuste, bartdesign"
        case .libserum:
            return "zesinger, mkalkbrenner, pinballpower, jsm174, vbousquet, toxieainc"
        case .libdof:
            return "jsm174, dynajoe, christiancoleman, dejaloomer, patsoffice, superhac, dekay"
        case .libvni:
            return "jsm174, mkalkbrenner, djrobx"
        case .libwinevbs:
            return "jsm174, francisdb, gitfool"
        case .artwork:
            return "smillard316 (Table placeholder), adam.co (App icon enhancements), twostraws (Shimmer metal shader)"
        default:
            return nil
        }
    }

    var link: Link? {
        switch self {
        case .vpinball:
            return .vpinball
        case .pinmame:
            return .pinmame
        case .libaltsound:
            return .libaltsound
        case .libdmdutil:
            return .libdmdutil
        case .libzedmd:
            return .libzedmd
        case .libserum:
            return .libserum
        case .libdof:
            return .libdof
        case .libvni:
            return .libvni
        case .libwinevbs:
            return .libwinevbs
        case .other:
            return .thirdparty
        default:
            return nil
        }
    }
}
