# Entrées et nudge en 10.8.1

[← Index](README.md#-français) · [🇬🇧 English](input_eng.md) · 🇫🇷 Français

La 10.8.1 a remplacé la gestion des entrées du plongeur et du nudge. Les
anciennes clés `Mapping.PlungerPos` / `Mapping.NudgeX1` ont disparu et ne sont
plus lues. Voici le schéma qui leur a succédé, et le chemin que parcourt une
mesure avant d'atteindre la bille.

## Périphériques

Chaque périphérique est déclaré dans `[Input]` :

```ini
Devices = SDLJoy_PSC0041701862884E45J009
Device.SDLJoy_PSC0041701862884E45J009.Type =
Device.SDLJoy_PSC0041701862884E45J009.Name =
Device.SDLJoy_PSC0041701862884E45J009.NoAutoLayout = 1
```

`NoAutoLayout = 1` empêche VPX de proposer sa disposition par défaut pour ce
périphérique. Ce détail compte plus qu'il n'y paraît : les mappings par défaut —
et donc leurs échelles d'unité — ne sont installés que par
`ApplyDefaultDeviceMapping`, qui ne s'exécute que si un périphérique est
auto-détecté **et** que la proposition de disposition est acceptée. Une
configuration qui contient déjà des mappings, ou qui pose `NoAutoLayout`,
conserve ce qu'elle a.

## Mappings de capteurs

Les capteurs sont comptés, puis mappés :

```ini
PlungerSensorCount = 1
Mapping.Plunger0.Position = <device>;514;P;0.000000;1.000000;1.000000
Mapping.Plunger0.Velocity = <device>;517;V;0.000000;12.500000;1.000000

NudgeSensorCount = 1
Mapping.Nudge0.AccX = <device>;512;A;0.030000;9.806650;1.000000
Mapping.Nudge0.AccY = <device>;513;A;0.030000;9.806650;1.000000
Mapping.Nudge0.Type = 1
Mapping.Nudge0.Strength = 1.000000
```

Chaque ligne comporte six champs :

```
device ; axe ; type ; deadZone ; scale ; limit
```

| Champ | Signification |
|---|---|
| `device` | l'identifiant déclaré plus haut |
| `axe` | numéro d'axe SDL |
| `type` | `P` position, `V` vitesse, `A` accélération |
| `deadZone` | fraction de la course annulée autour du repos, `0`–`0.3` |
| `scale` | **conversion d'unité** — voir plus bas |
| `limit` | écrêtage appliqué à la valeur normalisée |

Ils s'appliquent dans cet ordre, dans `SensorMapping::UpdateValue` :

1. la valeur SDL brute est normalisée entre −1 et +1 (÷ 32768) ;
2. la zone morte est retirée et le reste remis à l'échelle — `(v − dz) / (1 − dz)`,
   pour que le franchissement de la zone morte reparte de zéro au lieu de sauter ;
3. la valeur est écrêtée à ±`limit` ;
4. elle est multipliée par `scale`.

La zone morte mord donc **avant** l'échelle, sur la fraction d'axe brute. Les
mêmes 3 % avalent 0,29 m/s² sur une carte 1 g et 2,4 m/s² sur une carte 8 g.

## L'échelle est une unité, pas une sensibilité

`scale` convertit la valeur d'axe normalisée dans l'unité qu'attend le moteur
physique : **m/s² pour les accélérations, m/s pour les vitesses, unités/s pour le
plongeur** (l'unité étant la course du plongeur). Laissée à 1.0, une mesure à
fond d'échelle arrive au moteur comme 1 m/s² au lieu de 1 g, et l'impulsion du
plongeur ressort deux ordres de grandeur trop bas — le `/100` de
`HitPlunger::HitTest` est censé être absorbé par cette échelle.

Ce sont les valeurs que la page de configuration des capteurs propose en
préréglages :

| Capteur | Échelle | Sens |
|---|---|---|
| Accéléromètre ±1 g | `9.80665` | pleine déviation = 1 g |
| Accéléromètre ±2 g | `19.61330` | |
| Accéléromètre ±4 g | `39.22660` | |
| Accéléromètre ±8 g | `78.45320` | |
| Vitesse plongeur Pinscape | `12.5` | unités/s sur la course du plongeur |
| Vitesse nudge Pinscape | `4096 / (20 × 1000)` | la carte rapporte des mm/s ×20 sur ±4096 |

La sensibilité est un champ distinct : `Mapping.NudgeN.Strength`, de `0` à `2`
autour d'un `1` neutre. Piloter la sensibilité par `scale` revient à mentir au
moteur sur la nature du capteur, et rend une carte 4 g ou 8 g indescriptible.

## Laquelle des quatre échelles compte vraiment

Un cabinet peut mapper quatre axes : position et vitesse du plongeur,
accélération et vitesse du nudge. Ils ne se valent pas — deux seulement ont
besoin d'une échelle qui veut dire quelque chose physiquement.

| Mapping | Nécessaire ? | Échelle |
|---|---|---|
| `Plunger0.Position` | **obligatoire** pour que le plongeur fasse quoi que ce soit | à laisser à `1.0` — c'est une course normalisée 0…1, pas une unité physique |
| `Plunger0.Velocity` | facultatif | **doit convertir en m/s** (`12.5` sur Pinscape) |
| `Nudge0.AccX` / `AccY` | la référence physique | **doit convertir en m/s²** (`19.6133` pour une carte ±2 g) |
| `Nudge0.VelX` / `VelY` | facultatif | calibrée automatiquement — voir plus bas |

Sans mapping de position, `PlungerSensor::StepOneMillisecond` sort
immédiatement et tout le capteur de plongeur reste inerte. La position est
celle qu'on ne peut pas omettre, et celle dont l'échelle doit être laissée
telle quelle ; on n'inverse son signe que si l'axe se lit à l'envers.

La vitesse du plongeur est facultative, mais lorsqu'elle est mappée
`GetHitVelocity` renvoie **sa valeur directement** comme vitesse d'impact —
d'où une échelle qui doit être une vraie conversion en m/s. Non mappée, VPX
déduit la vitesse de l'estimateur de position, ce qu'obtient toute carte sans
canal de vitesse.

La vitesse du nudge se comporte encore autrement. Quand l'accélération *et* la
vitesse sont mappées sur le même axe, `CabinetNudgeSensor::UpdateAxis` pousse
les deux dans le même filtre de Kalman et laisse `MotionGainCalibratorAxis`
déterminer seul le gain entre les deux canaux, en prenant l'accélération pour
référence — la vitesse est injectée à `1 / gain`. Tant que cette calibration
n'atteint pas une confiance de 0,5, le canal de vitesse est purement
**ignoré** et seule l'accélération est utilisée. Son échelle n'a donc pas
besoin d'être exacte : la calibration l'absorbe. L'exception est de mapper la
vitesse *sans* accélération sur cet axe : elle est alors utilisée avec un gain
de 1 et son échelle compte pleinement (m/s).

En résumé pour un ini : corriger l'échelle de `Plunger0.Velocity` et de
`Nudge0.AccX/AccY`, laisser `Plunger0.Position` et `Nudge0.VelX/VelY` telles
qu'elles viennent.

## Les trois modes de nudge

`Mapping.NudgeN.Type` choisit comment une mesure devient un mouvement de cabinet :

**0 — Manette de jeu.** Pour un gamepad ou un stick VR. Lit l'amplitude et la
vitesse de poussée du stick pour en déduire un nudge. Sans rapport avec un
accéléromètre.

**1 — Capteur d'intention.** Pour les cabinets équipés d'une carte
accéléromètre. Votre accélération n'est pas transmise telle quelle :
`NudgeIntentHandler` détecte les pics, écarte les oscillations propres du
cabinet (un pic plus faible survenant dans les 300 ms après un plus fort), et
au-delà d'un seuil injecte une impulsion calibrée de 25 ms en demi-cosinus dont
l'amplitude est celle du pic. En dessous de ce seuil, **rien du tout** n'atteint
la bille. Conçu pour les cartes bruitées, biaisées ou lentes.

**2 — Capteur de cabinet.** Pour un capteur rapide, propre et à faible latence.
La mesure pilote directement le cabinet, lissée par une EMA de 4 ms et ramenée de
la masse réelle du cabinet (`Mapping.NudgeN.CabWeight`, en kg) aux 113 kg
simulés. Aucun seuil : les petites secousses comptent, le bruit du capteur aussi.

Le seuil d'intention vaut **1 m/s², codé en dur** dans
`NudgeIntentHandler::EvaluateImpulse`, et il est comparé *après* application de
`Strength`. Monter Strength rend donc les nudges à la fois plus forts et plus
faciles à reconnaître. Ce seuil n'est exposé dans aucune interface.

## La chaîne complète

Pour un accéléromètre, dans l'ordre :

```
axe brut → zone morte → limit → scale (m/s²)
  → estimateur de Kalman (toujours, dans les deux modes)
  → × Strength
  → mode : seuil d'intention + impulsion   |   EMA + poids du cabinet
  → oscillateur amorti du cabinet (113 kg ; 9,3 Hz ζ0,052 en X, 5,8 Hz ζ0,055 en Y)
  → pendule (tige de 100 mm, 1 kHz)
  → tilt lorsque la tige dépasse PlumbThresholdAngle
```

L'estimateur de Kalman (`MotionKalmanAxis`) n'est pas un lisseur : son rôle est
de retirer le biais, pour qu'une carte montée à un degré près ne se lise pas
comme une poussée permanente. Une conséquence mérite d'être connue : **il prend
son tout premier échantillon pour du biais**. Un capteur qui bouge avant d'avoir
jamais été au repos est lu comme un décalage, pas comme un mouvement.

## Tilt

Deux clés dans `[Player]` :

| Clé | Plage | Défaut | Sens |
|---|---|---|---|
| `PlumbThresholdAngle` | 0,15–4 ° | 1,0 | angle de tige qui déclenche le tilt |
| `PlumbDamping` | 0–2 | 1,0 | 0 oscille sans fin, 2 se stabilise deux fois plus vite que la réalité |

VPX ne compare pas une accélération à un seuil. Il simule une masse au bout
d'une tige de 100 mm, entraînée par la gravité et l'accélération du cabinet,
intégrée toutes les millisecondes, et déclenche le tilt quand l'angle de la tige
dépasse le seuil. Un coup bref et violent peut la laisser immobile là où trois
tapes rythmées la font partir — ce qu'aucun seuil instantané ne sait exprimer.

La secousse qu'exige un seuil n'a rien d'évident, et dépend du mode. En simulant
la chaîne ci-dessus avec une poussée de 100 ms, à Strength 1.0, voici
l'accélération que le capteur doit voir :

| Seuil | Intention | Cabinet |
|---|---|---|
| 0,15° | 1,0 m/s² | 0,5 m/s² |
| 1,00° (défaut) | 6,7 m/s² | 3,5 m/s² |
| 2,08° | 14,0 m/s² | 7,4 m/s² |
| 4,00° | 27,2 m/s² | 14,5 m/s² |

Deux conséquences. Le mode intention exige environ deux fois la secousse du mode
cabinet pour le même angle, puisque seul le pic de votre coup survit à
l'impulsion de 25 ms. Et sur une carte ±1 g — pleine échelle 9,81 m/s² — tout ce
qui dépasse environ 1,4° en mode intention est **hors d'atteinte** : le capteur
sature avant que le pendule n'y parvienne, si bien que les deux tiers hauts du
curseur ne font rien.

## Sources

- `src/input/SensorMapping.h` — les six champs, l'ordre de traitement, les échelles
- `src/input/InputManager.cpp` — déclaration des périphériques, `ApplyDefaultDeviceMapping`
- `src/physics/cabinet/NudgeIntentHandler.cpp` — détection de pic, impulsion, seuil
- `src/physics/cabinet/CabinetNudgeSensor.cpp` — le chemin du capteur de cabinet
- `src/physics/cabinet/MotionKalmanAxis.h` — l'estimateur et son hypothèse de biais
- `src/physics/cabinet/PlumbHandler.cpp` — le pendule
