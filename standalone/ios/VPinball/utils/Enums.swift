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
    case thirdparty

    var url: URL {
        switch self {
        case .docs:
            return URL(string: "https://github.com/vpinball/vpinball/blob/master/standalone/docs/ios/README.md")!
        case .troubleshooting:
            return URL(string: "https://github.com/vpinball/vpinball/blob/master/standalone/docs/ios/README.md#troubleshooting")!
        case .discord:
            return URL(string: "https://discord.com/channels/652274650524418078/1076655472846831667")!
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
            return URL(string: "https://github.com/ppuc/libzedmd")!
        case .libserum:
            return URL(string: "https://github.com/zesinger/libserum")!
        case .libdof:
            return URL(string: "https://github.com/jsm174/libdof")!
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
    case other
    case artwork

    static let all: [Credit] = [.vpinball,
                                .pinmame,
                                .libaltsound,
                                .libdmdutil,
                                .libzedmd,
                                .libserum,
                                .libdof,
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
        case .other:
            return "Other third party libraries"
        case .artwork:
            return "Artwork"
        }
    }

    var authors: String? {
        switch self {
        case .vpinball:
            return "toxieainc, vbousquet, fuzzelhjb, jsm174, c-f-h, bcd, cupidsf, djrobx, brandrew2, francisdb, koadic76, shagendo, horseyhorsey, CraftedCart, snail_gary, Matthias, Buecher, Nicals, YellowLabrador, markmon, nkissebe, mkalkbrenner, freezy, WildCoder, randydavis99, ntleverenz, latsao, Wylted1, WizardsHat, ScaryG, Chickenzilla, ravarcade, poiuyterry, omigeot, mjrgh, manofwar32, kaicherry, droscoe, cschmidtpxc, RockfordRoe, JockeJarre, CapitaineSheridan, Billiam, LeHaine, claytgreene"
        case .pinmame:
            return "volkenborn, toxieainc, Steve Ellenoff, bcd, Tom Haukap, wpcmame, Matthias Buecher, jsm174, mkalkbrenner, droscoe, vbousquet, djrobx, Thomas Behrens, bontango, mjrgh, Oliver Kaegi, syllebra, gstellenberg, Randall, Perlow, JockeJarre, tomlogic, Netsplits, gnulnulf, freezy, Sunnucks, Mark, uid68989, francisdb, diego-link-eggy, Sereda, Pavel, tomlogic"
        case .libaltsound:
            return "droscoe, jsm174, toxieainc, francisdb"
        case .libdmdutil:
            return "mkalkbrenner, jsm174, toxieainc, bartdesign, francisdb, freezy"
        case .libzedmd:
            return "mkalkbrenner, jsm174, zesinger, bartdesign"
        case .libserum:
            return "zesinger, mkalkbrenner, pinballpower, jsm174, toxieainc"
        case .libdof:
            return "zesinger, mkalkbrenner, jsm174"
        case .artwork:
            return "smillard316 (Table placeholder), adam.co (App icon enhancements), Juan Pablo Bravo (The Noun Project 17955), Zach Bogart (The Noun Project 3169564), Templarian (SVG Repo 369724), twostraws (Shimmer metal shader)"
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
        case .other:
            return .thirdparty
        default:
            return nil
        }
    }
}
