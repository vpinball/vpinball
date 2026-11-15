import SwiftUI

enum TableGridSize: Int, Hashable {
    case small = 0
    case medium = 1
    case large = 2

    func columns(width: CGFloat, height: CGFloat, spacing: CGFloat, aspect: CGFloat) -> Int {
        let (minimum, cardWidth, heightFraction): (Int, CGFloat, CGFloat) = switch self {
        case .small: (4, 100, 0.5)
        case .medium: (3, 150, 0.66)
        case .large: (2, 225, 0.9)
        }
        let target = min(cardWidth, height * heightFraction * aspect)
        guard width > 0, target > 0 else { return minimum }
        return max(minimum, Int(((width + spacing) / (target + spacing)).rounded()))
    }
}

enum AppAppearance: Int, CaseIterable, Identifiable {
    case dark = 0
    case light = 1
    case system = 2

    var id: Int {
        rawValue
    }

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
            return "toxieainc, vbousquet, fuzzelhjb, jsm174, francisdb, c-f-h, bcd, cupidsf, gitfool, djrobx, brandrew2, mjrgh, koadic76, Le-Syl21, superhac, shagendo, CraftedCart, horseyhorsey, Nicals, kara2010, snail_gary, cwick, Matthias Buecher, baxelrod-bdai, claytgreene, YellowLabrador, dcbenji, JockeJarre, markmon, mkalkbrenner, SPD13, BRoadifer, freezy, nkissebe, ScaryG, WildCoder, andremichi, Chickenzilla, dekay, hughfitzgerald, jayadelson, latsao, ntleverenz, omigeot, Pyrrvs, RandyDavis2000, RockfordRoe, WizardsHat, Wylted1, Billiam, CapitaineSheridan, colas-sebastien, cschmidtpxc, droscoe, dynajoe, evilwraith, garybrowndev, hamidzr, Herschel, jermatic1, joni999, kaicherry, KutsuyaYuki, LeHaine, manofwar32, mcragun, nicolaspr56, poiuyterry, qinghao1, ravarcade, surtarso, teamsuperpanda, Yuki"
        case .pinmame:
            return "toxieainc, volkenborn, Steve Ellenoff, bcd, Tom Haukap, wpcmame, Matthias Buecher, vbousquet, gerwout, jsm174, mkalkbrenner, Le-Syl21, droscoe, djrobx, tomlogic, Thomas Behrens, bontango, mjrgh, Oliver Kaegi, whaslbeck, syllebra, JockeJarre, evilwraith, gitfool, BRoadifer, francisdb, gstellenberg, Randall Perlow, freezy, jayadelson, Netsplits, bhitney, gnulnulf, Disservin, Mark Sunnucks, mattwalsh, qinghao1, Samasaur1, coryaltheide, diego-link-eggy, Herschel, joni999, kara2010, noflip95, Pavel Sereda, uid68989"
        case .libaltsound:
            return "jsm174, toxieainc, Le-Syl21, freezy, francisdb, gitfool"
        case .libdmdutil:
            return "mkalkbrenner, jsm174, toxieainc, Al Linke, bartdesign, djrobx, francisdb, PastorL69"
        case .libzedmd:
            return "mkalkbrenner, jsm174, zesinger, PastorL69, bartdesign, Cpasjuste"
        case .libserum:
            return "zesinger, mkalkbrenner, pinballpower, jsm174, toxieainc, vbousquet"
        case .libdof:
            return "jsm174, dynajoe, philipellisis, christiancoleman, dejaloomer, dekay, kbvandergriffjr, patsoffice, superhac"
        case .libvni:
            return "jsm174, mkalkbrenner, djrobx"
        case .libwinevbs:
            return "jsm174, toxieainc, francisdb, gitfool"
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
