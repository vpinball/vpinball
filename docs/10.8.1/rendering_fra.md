# Rendu en 10.8.1

[← Index](README.md#-français) · [🇬🇧 English](rendering_eng.md) · 🇫🇷 Français

Tous ces réglages vivent dans `[Player]`. Le travail sur le frame pacing
([`3da46f1ef`](https://github.com/vpinball/vpinball/commit/3da46f1ef)) a remplacé
les réglages de latence hérités de DX9 — voir
[Supprimés](removed_fra.md#rendu).

## Synchronisation

`SyncMode` décide de la cadence de l'image, et son défaut a changé : **le frame
pacing est désormais le mode par défaut**, plus la synchronisation verticale.

| Valeur | Mode | Compromis |
|---|---|---|
| `0` | Aucune | déchirure, latence minimale |
| `1` | Synchro verticale | pas de déchirure, forte latence visuelle |
| `2` | Synchro adaptative | vsync sauf pour les images en retard, latence encore élevée |
| `3` | **Frame pacing** (défaut) | cadence le rendu pour réduire la latence ; peut saccader sur une machine trop juste |

`MaxFramerate` limite la cadence, et ses deux valeurs spéciales méritent d'être
connues :

| Valeur | Effet |
|---|---|
| `-1` (défaut) | limiter à la fréquence de rafraîchissement de l'écran |
| `0` | aucune limite |
| toute autre | limiter à cette valeur — cadence plus stable, moins de chaleur |

Notez l'inversion : `0` ne veut pas dire « pas de bridage raisonnable », il veut
dire « aucune limite du tout », tandis que `-1` est le défaut sensé. Lire ces
valeurs comme de simples nombres conduit à l'inverse de l'intention.

## Anticrénelage

Trois étages indépendants, qui se cumulent :

| Clé | Valeurs | Rôle |
|---|---|---|
| `AAFactor` | 0,5–2,0, défaut 1,0 | suréchantillonnage — rend à ce multiple de la résolution puis réduit. 2,0 = quatre fois plus de pixels |
| `MSAASamples` | `0` désactivé, `1` 4×, `2` 6×, `3` 8× | crénelage géométrique seulement, coûte de la mémoire GPU |
| `FXAA` | `0` désactivé … `7` | post-traitement : FXAA rapide/standard/qualité, NFAA rapide, DLAA standard, SMAA qualité, FAAA qualité |

`AAFactor` est la force brute, et de loin le plus coûteux : 200 % double la
résolution dans chaque direction, soit quatre fois le travail. Le MSAA est moins
cher mais ne touche que les arêtes géométriques. Le FXAA est presque gratuit et
adoucit un peu l'image.

`Sharpen` (`0` désactivé, `1` CAS, `2` CAS bilatéral) existe surtout pour
contrer cet adoucissement.

## Réflexions et textures

| Clé | Valeurs | Rôle |
|---|---|---|
| `PFReflection` | `0`–`5`, défaut `5` | Désactivé / Billes seules / Statique seul / Statique & billes / Statique & dynamique non synchronisé / **Dynamique** |
| `SSRefl` | booléen | réflexions en espace écran |
| `MaxTexDimension` | 512–16384, défaut 16384 (1536 sur mobile) | les images plus grandes sont réduites au chargement |

`PFReflection` à `2` (statique seul) est le seul sans coût à l'exécution, sauf en
VR. `5` est le défaut et le plus beau.

## Stéréo

`Stereo3D` choisit le mode, et les modes factices ont disparu
([`7c456c07a`](https://github.com/vpinball/vpinball/commit/7c456c07a)) — il ne
reste que la vraie stéréo : haut/bas, variantes entrelacées, côte à côte,
anaglyphe. `Stereo3DEnabled` l'active, et l'écartement des yeux, la luminosité,
la saturation, le flou et le contraste par œil l'ajustent.

## Le réglage cosmétique à connaître

`BallAntiStretch` (« Unstretch Ball », défaut **inactif**) compense l'étirement
de rendu qui donne à une bille rapide une allure d'œuf. Il ne change rien à la
physique — mais sur un plateau de cabinet, on le veut généralement, et il reste
inactif tant qu'on ne le demande pas.

## Sources

- `src/core/Settings_properties.inl` — toutes les clés ci-dessus, avec plages et défauts
- `src/renderer/Renderer.cpp` — assemblage de la chaîne de post-traitement
