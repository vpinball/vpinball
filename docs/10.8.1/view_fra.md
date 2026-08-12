# Vue et ajustement cabinet en 10.8.1

[← Index](README.md#-français) · [🇬🇧 English](view_eng.md) · 🇫🇷 Français

La 10.8.1 ajoute l'ajustement automatique de la vue pour les cabinets, et avec
lui une dépendance que la plupart des tables n'ont jamais eu à satisfaire : la
hauteur de la vitre.

## Trois projections

`ViewLayoutMode` décide de la façon dont la scène est projetée. Il se règle par
mode de vue et par table, et se trouve en jeu dans **F12 → Point of View** :

| Mode | Effet |
|---|---|
| `0` Legacy | rendu d'avant la 10.8 — un cadrage sur des sommets englobants, biaisé par un angle de *layback*. Des déformations visuellement fausses, par construction. |
| `1` Camera | perspective classique, spectateur placé par rapport au bas-centre de la table. Le mode desktop. |
| `2` Window | reprojection oblique, spectateur placé par rapport au bas-centre de **l'écran**. Conçu pour les cabinets. |

Window est celui qui traite votre écran comme la vitre physique du cabinet, avec
le spectateur positionné par rapport à elle en unités réelles. C'est aussi
pourquoi le head tracking n'y fonctionne que là : déplacer l'œil doit produire un
frustum asymétrique, ce qui suppose un écran existant dans le modèle.

Les clés sont `TableOverride.ViewCabMode`, `ViewDTMode` et `ViewFSSMode` — une
par mode de vue (Cabinet, Desktop, FSS), enregistrées comme surcharge par table
plutôt que dans les réglages de l'application.

## Ajustement automatique

`Player.CabinetAutofitMode` s'applique à la vue Cabinet :

| Valeur | Nom | Effet |
|---|---|---|
| `0` | Manual | rien n'est calculé ; le point de vue propre à la table est utilisé |
| `1` | Fit Table | échelle uniforme, calée sur la largeur de la table |
| `2` | Fit Screen | étirement non uniforme, la table remplit l'écran |

La différence tient en deux lignes de `ViewSetup::SetWindowAutofit` :

```cpp
mSceneScaleX = (screenHeight / table->GetTableWidth()) * (table->GetHeight() / screenWidth);
mSceneScaleY = allowNonUniformStretch ? 1.f : mSceneScaleX;
```

**Fit Table** conserve les proportions — Y est mis à l'échelle comme X — au prix
de ne pas tout montrer : une partie du tablier, ou du haut de la table, sort du
cadre. `Player.CabinetAutofitPos` (0–20 %, défaut 5 %) décide alors de la
position à l'écran des palettes au repos, afin que le même point de référence
vaille d'une table à l'autre.

**Fit Screen** ne coupe rien mais étire un axe par rapport à l'autre. Sur un
cabinet avec head tracking, c'est le mauvais choix : la parallaxe suppose que la
géométrie rendue corresponde à la géométrie réelle, et une scène étirée répond
différemment à un mouvement de tête horizontal et vertical.

Choisir l'un ou l'autre mode force `mMode = VLM_WINDOW` : l'ajustement
automatique et la projection Window vont donc ensemble.

## Ce que l'ajustement attend de la table

Deux choses, dont une seule dépend de la table.

**Les dimensions physiques de l'écran**, `Player.ScreenWidth` et
`Player.ScreenHeight`, en centimètres. L'ajustement refuse de s'exécuter sans
elles :

```
Screen dimensions must be defined before using automatic point of view
```

Ce sont les dimensions de la zone visible du plateau, pas la diagonale de la
dalle. Fausses, elles décadrent toutes les tables de la même façon.

**Les hauteurs de vitre**, `m_glassBottomHeight` et `m_glassTopHeight`, qui
deviennent le volume de projection :

```cpp
Vertex2D glass = table->EvaluateGlassHeight();
if (table->m_glassTopHeight != table->m_glassBottomHeight) { /* la table les déclare */ }
else { glassNotification("Missing glass position guessed to be {..}cm / {..}cm"); }
mWindowBottomZOfs = bottomHeight;
mWindowTopZOfs    = topHeight;
```

Une vitre horizontale était l'ancien réglage par défaut : une table qui ne les a
jamais définies se reconnaît donc à sa vitre plate, et VPX **devine** les deux
hauteurs en analysant les bornes des éléments. Lorsque la table les déclare, VPX
utilise ses valeurs mais les compare à sa propre estimation et prévient si
l'écart dépasse un pouce :

```
Glass height was evaluated to X cm / Y cm
It differs from the defined glass position Z cm / W cm
```

Ces deux notifications sont la première chose à lire quand une table tombe mal en
mode Window. Elles disent si le cadrage repose sur une géométrie déclarée ou sur
une supposition.

## Pourquoi des tables tombent mal

Le mode Window met à l'épreuve une géométrie que Camera et Legacy n'ont jamais
sollicitée. Une table peut être parfaite en desktop et présenter, sur un
cabinet :

- **un tablier ou des parois de cabinet dessinés là où ils ne devraient pas
  l'être** — une géométrie qui n'avait jamais été vue que d'un point de vue qui
  la masquait ;
- **des éléments décalés proportionnellement à leur hauteur** au-dessus du
  plateau, ce qui signe un volume de projection faux plutôt que des pièces mal
  placées : un insert peint ne peut pas bouger, une rampe à 10 cm bouge le plus ;
- **un cadrage qui coupe trop ou pas assez**, lorsque les hauteurs de vitre ont
  été devinées au lieu d'être déclarées.

Rien de tout cela n'est visible sans un cabinet en mode Window, et c'est
précisément pourquoi c'est passé inaperçu : ceux qui pourraient corriger les
tables sont ceux qui ont le moins de chances de croiser le problème.

## Sources

- `src/renderer/ViewSetup.h` — `ViewLayoutMode`, les trois projections
- `src/renderer/ViewSetup.cpp` — `SetWindowAutofit`, hauteurs de vitre, échelles de scène
- `src/core/player.cpp` — `SetCabinetAutoFitMode`, `SetCabinetAutoFitPos`
- `src/ui/live/ingameui/PointOfViewSettingsPage.cpp` — la page F12
- `src/core/Settings_properties.inl` — `CabinetAutofitMode`, `CabinetAutofitPos`, `BGSet`
