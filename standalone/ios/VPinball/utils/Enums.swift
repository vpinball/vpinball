import SwiftUI

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
    case thirdparty

    var url: URL {
        switch self {
        case .docs:
            return URL(string: "https://github.com/vpinball/vpinball/blob/master/standalone/docs/ios/README.md")!
        case .troubleshooting:
            return URL(string: "https://github.com/vpinball/vpinball/blob/master/standalone/docs/ios/README.md#troubleshooting")!
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
            return URL(string: "https://github.com/jsm174/libdof")!
        case .libvni:
            return URL(string: "https://github.com/PPUC/libvni")!
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
        case .other:
            return "Other third party libraries"
        case .artwork:
            return "Artwork"
        }
    }

    var authors: String? {
        switch self {
        case .vpinball:
            return "toxieainc, vbousquet, fuzzelhjb, jsm174, c-f-h, francisdb, bcd, cupidsf, djrobx, brandrew2, mjrgh, koadic76, shagendo, Nicals, horseyhorsey, CraftedCart, superhac, snail_gary, Matthias Buecher, Le-Syl21, baxelrod-bdai, YellowLabrador, claytgreene, markmon, JockeJarre, WildCoder, ScaryG, nkissebe, mkalkbrenner, freezy, Wylted1, WizardsHat, RandyDavis2000, ntleverenz, latsao, Chickenzilla, Yuki, teamsuperpanda, surtarso, RockfordRoe, ravarcade, poiuyterry, omigeot, manofwar32, LeHaine, KutsuyaYuki, kaicherry, joni999, jmarzka, droscoe, cschmidtpxc, CapitaineSheridan, Billiam, andremichi, evilwraith"
        case .pinmame:
            return "toxieainc, volkenborn, Steve Ellenoff, bcd, Tom, Haukap, wpcmame, Matthias Buecher, jsm174, vbousquet, mkalkbrenner, droscoe, djrobx, Thomas Behrens, bontango, tomlogic, mjrgh, Oliver, Kaegi, syllebra, JockeJarre, Randall, Perlow, gstellenberg, Netsplits, gnulnulf, Sunnucks, mattwalsh, Mark, freezy, uid68989, Sereda, Pavel, noflip95, No, francisdb, diego-link-eggy"
        case .libaltsound:
            return "droscoe, jsm174, toxieainc, francisdb"
        case .libdmdutil:
            return "mkalkbrenner, jsm174, toxieainc, francisdb, bartdesign, freezy"
        case .libzedmd:
            return "mkalkbrenner, jsm174, zesinger, Cpasjuste, bartdesign"
        case .libserum:
            return "zesinger, mkalkbrenner, pinballpower, jsm174, vbousquet, toxieainc"
        case .libdof:
            return "jsm174, dekay"
        case .libvni:
            return "mkalkbrenner, freezy, jsm174"
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
        case .other:
            return .thirdparty
        default:
            return nil
        }
    }
}
