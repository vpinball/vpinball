# VR en 10.8.1

[← Index](README.md#-français) · [🇬🇧 English](vr_eng.md) · 🇫🇷 Français

OpenVR a été supprimé en juin 2026
([`6524789e3`](https://github.com/vpinball/vpinball/commit/6524789e3)) : la VR
repose désormais sur OpenXR seul. Les réglages vivent dans `[PlayerVR]`, et les
options VR qui se trouvaient dans les boîtes de dialogue Win32 sont passées dans
l'interface en jeu
([`be980d774`](https://github.com/vpinball/vpinball/commit/be980d774)).

## Activation

| Clé | Valeurs | Défaut | Rôle |
|---|---|---|---|
| `AskToTurnOn` | `0` activé, `1` détection auto, `2` désactivé | `0` sur les builds compatibles VR, `2` ailleurs | utiliser ou non la VR |

## Placer le cabinet

Ce sont les réglages qui font qu'un cabinet VR paraît juste, et ils sont tous
surchargeables par table :

| Clé | Plage | Défaut | Rôle |
|---|---|---|---|
| `Orientation` | −180…180 ° | 0 | rotation de la vue |
| `TableX`, `TableY`, `TableZ` | −100…100 | 0 | décalages de position |
| `LockFeetToGround` | booléen | actif | garde les pieds du cabinet au sol au lieu de le laisser flotter |
| `AddBackglass` | booléen | inactif | ajoute un backglass par défaut à la scène |

`LockFeetToGround` est celui qui change le plus l'impression : sans lui, le
cabinet semble léviter, ce qui paraît faux même quand la hauteur est bonne.

## Centrage par les manettes

Plutôt que d'ajuster les décalages à la main, la vue peut être centrée en
pointant les manettes vers le cabinet réel. Deux réglages calibrent ce
mécanisme :

| Clé | Plage | Défaut | Rôle |
|---|---|---|---|
| `ControllerCabYOffset` | −150…50 | 0 | décalage en Y appliqué lors du centrage aux manettes |
| `ControllerLockbarScale` | 0,5–2,0 | 1,0 | rapport de taille du lockbar utilisé par ce centrage |

## La fenêtre d'aperçu

La VR dessine un aperçu plat sur le bureau, et c'est une fenêtre comme une autre
— les mêmes onze clés que les sorties décrites dans
[Fenêtres](window_fra.md#quatre-sorties-un-seul-schéma), préfixées `Preview` :
`PreviewDisplay`, `PreviewFullScreen`, `PreviewWndX/Y`, `PreviewWidth/Height`,
`PreviewFSWidth/FSHeight`, `PreviewRefreshRate`, `PreviewColorDepth`.

`ShrinkPreview` (inactif par défaut) le réduit, ce qui coûte moins cher que de le
rendre en taille réelle.

## Sources

- `src/core/Settings_properties.inl` — le bloc `[PlayerVR]`
- `src/ui/live/ingameui/` — les pages VR, depuis la disparition des dialogues Win32
