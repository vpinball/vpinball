# Réglages supprimés en 10.8.1

[← Index](README.md#-français) · [🇬🇧 English](removed_eng.md) · 🇫🇷 Français

Les réglages qui existaient en 10.8 et n'existent plus. VPX les ignore
silencieusement : un `ini` repris de la 10.8 conserve donc des lignes qui ont
l'air de vouloir dire quelque chose et ne font rien.

Chaque entrée indique ce que faisait le réglage, le commit qui l'a supprimé, et
ce qui l'a remplacé — ou le fait que rien ne l'a remplacé, ce qui est justement
ce qu'il faut savoir avant de migrer une configuration.

## Entrées

Le système d'entrée a été réécrit deux fois : en mars 2025
([`954ceb8d2`](https://github.com/vpinball/vpinball/commit/954ceb8d2)) puis en
octobre
([`f6252e874`](https://github.com/vpinball/vpinball/commit/f6252e874)). Les axes
ne sont plus des réglages nommés : ce sont des champs d'un mapping de capteur —
voir [Entrées](input_fra.md#mappings-de-capteurs).

| Supprimé | Rôle | Remplacé par |
|---|---|---|
| `LRAxis`, `UDAxis` | numéros d'axe de l'accéléromètre | le champ axe de `Mapping.NudgeN.AccX` / `AccY` |
| `LRAxisFlip`, `UDAxisFlip` | inversion d'axe | un `scale` négatif dans le mapping |
| `PlungerAxis`, `PlungerSpeedAxis` | numéros d'axe du plongeur | `Mapping.Plunger0.Position` / `.Velocity` |
| `ReversePlungerAxis` | inversion du plongeur | un `scale` négatif dans le mapping |
| `AccelVelocityInput` | « la carte envoie une vitesse, pas une accélération » | le champ type du mapping : `V` au lieu de `A` |
| `EnableNudgeFilter` | filtre anti-bruit sur le signal brut | le champ de zone morte, plus l'estimateur de Kalman que VPX applique désormais toujours |
| `PlungerFilter` | jamais implémenté | rien ([`5b690a030`](https://github.com/vpinball/vpinball/commit/5b690a030)) |
| `InputApi` | choix du backend d'entrée | rien — SDL uniquement |
| `EnableMouseInPlayer` | autoriser la souris en jeu | rien |
| `DisableESC`, `EscapeKey` | désactiver ou remapper la touche Échap | un mapping d'action ordinaire, remappable dans **F12 → Input** |
| `Joy*Key` (tous) | un réglage par bouton de joystick | `Mapping.<Action>` avec un identifiant de périphérique et un numéro de bouton |

## Tilt et nudge

Le tilt historique a été supprimé en novembre 2025
([`89527dd2b`](https://github.com/vpinball/vpinball/commit/89527dd2b)) ; le
commit précise qu'il était désactivé depuis des années. La simulation du pendule
l'a remplacé — voir [Entrées](input_fra.md#tilt).

| Supprimé | Rôle | Remplacé par |
|---|---|---|
| `TiltAmount`, `TiltTriggerTime` | sensibilité et fenêtre du tilt historique | `PlumbThresholdAngle` |
| `JoltAmount`, `JoltTriggerTime` | détection de secousse historique | rien — le pendule couvre le cas |
| `PlumbInertia` | inertie du pendule | `PlumbDamping`, qui est un facteur d'amortissement et non une inertie |
| `EnableLegacyNudge`, `LegacyNudgeStrength` | modèle de nudge VP9 | `KeyboardNudgeMode` (qui propose toujours le modèle VP9) et `KeyboardNudgeStrength` |

## Rendu

| Supprimé | Rôle | Remplacé par | Commit |
|---|---|---|---|
| `Stereo3DFake`, `Stereo3DMaxSeparation`, `Stereo3DOffset`, `Stereo3DYAxis`, `Stereo3DZPD` | fausse stéréo, une approximation 2D | rien — stéréo réelle uniquement, via `Stereo3D` | [`7c456c07a`](https://github.com/vpinball/vpinball/commit/7c456c07a) |
| `Anaglyph` | anaglyphe comme mode séparé | un mode de `Stereo3D`, avec sa calibration dans F12 | [`7c456c07a`](https://github.com/vpinball/vpinball/commit/7c456c07a) |
| `ScaleFX`, `ScaleFXDMD` | agrandisseur ScaleFX | rien | [`8d018bbec`](https://github.com/vpinball/vpinball/commit/8d018bbec) |
| `AdaptiveVSync`, `EnableLegacyMaximumPreRenderedFrames`, `MaxPrerenderedFrames` | réglages de cadence hérités de DX9 | `SyncMode` (quatre valeurs, dont le frame pacing) et `MaxFramerate` | [`0f556f09b`](https://github.com/vpinball/vpinball/commit/0f556f09b) |
| `DisableDWM` | désactiver le compositeur Windows | rien | [`9cad8a131`](https://github.com/vpinball/vpinball/commit/9cad8a131) |
| `BWRendering` | rendu en noir et blanc | rien | [`da11f5481`](https://github.com/vpinball/vpinball/commit/da11f5481) |

## Fenêtres externes

Le backglass, le DMD et les fenêtres PUP appartenaient à la build standalone,
chacun avec ses propres réglages de position et de taille. Ils sont devenus des
plugins et des surfaces de rendu
([`779f7be15`](https://github.com/vpinball/vpinball/commit/779f7be15),
[`25ec2f640`](https://github.com/vpinball/vpinball/commit/25ec2f640),
[`6f0e23fe0`](https://github.com/vpinball/vpinball/commit/6f0e23fe0)).

| Supprimé | Remplacé par |
|---|---|
| `B2SBackglassX/Y/Width/Height/Rotation` | la section de fenêtre `[Backglass]` |
| `B2SDMDX/Y/Width/Height/Rotation/FlipY` | la section de fenêtre `[ScoreView]` |
| `B2SWindows`, `B2SPlugins`, `B2SDualMode`, `B2SHide*` | `[Plugin.B2SLegacy]` |
| `PUPWindows`, `PUPCapture`, `PUPPlugin` | `[Plugin.PUP]` |
| `DOFPlugin` | `[Plugin.DOF]` |
| `DMDServer`, `DMDServerAddr`, `DMDServerPort` | les réglages propres au plugin DMDUtil |
| `AltColor`, `AltSound` | `[Plugin.AltColor]`, `[Plugin.AltSound]` |

## À propos de la migration de novembre 2025

Une série de commits intitulés *Migrate … props* a déplacé chaque réglage vers un
registre de propriétés. Les noms ont pour l'essentiel survécu, mais certains ont
changé de section, et les valeurs par défaut ont été redéclarées en un seul
endroit — `src/core/Settings_properties.inl`, qui fait désormais autorité sur ce
qui existe.

Un réglage absent de ce fichier n'existe pas, quoi qu'en disent un ancien ini ou
un ancien guide.
