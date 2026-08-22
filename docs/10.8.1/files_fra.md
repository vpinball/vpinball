# Fichiers et dossiers en 10.8.1

[← Index](README.md#-français) · [🇬🇧 English](files_eng.md) · 🇫🇷 Français

L'endroit où VPX lit et écrit est décidé par `FileLocator`, et la règle qui
surprend est que **la disposition des fichiers n'est pas un réglage**. Elle est
déduite à chaque démarrage de l'endroit où se trouve `VPinballX.ini`.

## Les deux dispositions

`FileLocator::UpdateFileLayoutMode` cherche l'ini à deux endroits, dans cet
ordre :

| Ini trouvé dans | Disposition | Conséquence |
|---|---|---|
| le dossier de préférences | `AppPrefData` (défaut) | réglages et journal dans le dossier de préférences, fichiers par table à côté du `.vpx` |
| à côté de l'exécutable | `AppOnly` (héritée) | tout ensemble dans le dossier de l'application |
| ni l'un ni l'autre | `AppPrefData` | un dossier de préférences neuf est créé |

L'ordre compte : un ini présent dans le dossier de préférences **l'emporte**.
Poser une copie à côté de l'exécutable ne change donc rien à lui seul — pour
revenir à la disposition en un seul dossier, il faut que la copie du dossier de
préférences ait disparu, pas seulement qu'elle soit doublée.

`AppOnly` est le comportement d'avant la 10.8, hérité d'une époque où une
application pouvait écrire n'importe où. Il fonctionne toujours, et il exige un
accès en écriture au dossier d'installation.

## Où va quoi

Le dossier de préférences est `SDL_GetPrefPath(nullptr, "VPinballX")` suivi
d'un sous-dossier `majeure.mineure` — soit `~/.local/share/VPinballX/10.8/` sous
Linux, `%AppData%\VPinballX\10.8\` sous Windows. Le regroupement par version
mineure est volontaire : une révision mineure n'a pas le droit d'exiger une
reconfiguration.

| Contenu | `AppPrefData` | `AppOnly` |
|---|---|---|
| `VPinballX.ini` | dossier de préférences | dossier de l'application |
| `vpinball.log` | dossier de préférences | dossier de préférences |
| `assets/`, `scripts/`, `plugins/`, `docs/`, `shaders-x.y.z/` | dossier d'installation, lecture seule | idem |
| `user/` — scores, données sauvegardées par table | à côté du `.vpx` | partagé, dans le dossier de l'application |
| `music/` — utilisé par `PlayMusic()` | à côté du `.vpx` | dossier de l'application |
| `cache/` — textures décodées | `cache/<titre de la table>/` à côté du `.vpx` | dossier de préférences |
| `autosave/` | à côté du `.vpx` | dossier de l'application |

Que les fichiers d'une table atterrissent à côté d'elle plutôt que dans un tas
commun est justement ce qu'apporte la disposition moderne : elle donne son
meilleur quand chaque table a son propre dossier, et elle survit au
déplacement d'une table.

## Le dossier `user`

Il n'y a rien à créer à la main. VPX crée `user/` la première fois qu'une table
y écrit vraiment — un score, une option de table enregistrée — et le signale
dans le journal. Une installation neuve n'a donc simplement pas encore de
dossier `user/`, ce qui est normal et ne signale pas une installation cassée.

En venant d'une installation plus ancienne, un dossier `user/` partagé reste
utilisable : il est sur le chemin de lecture qu'on le pose à côté du binaire ou
dans le dossier de préférences.

## Où les scripts sont cherchés

`FileLocator::SearchScript` parcourt ces emplacements dans l'ordre et prend le
premier trouvé (sans tenir compte de la casse) :

1. le dossier de la table, puis `<table>/user`, puis `<table>/scripts` ;
2. le dossier de l'application, puis ses sous-dossiers `user`, `scripts` et
   `tables` ;
3. le dossier de préférences, puis ses sous-dossiers `user` et `scripts`.

## La migration au premier démarrage

Quand le dossier de préférences de cette `majeure.mineure` n'existe pas encore,
VPX le construit plutôt que de démarrer à vide. Il cherche, dans l'ordre :

1. le dossier de préférences d'une version précédente — en descendant de la
   version courante jusqu'à la 10.0, puis le dossier `VPinballX` sans version
   utilisé avant la disposition `majeure.mineure` — et en copie tout le contenu
   récursivement ;
2. à défaut, un `VPinballX.ini` posé à côté de l'exécutable, qu'il recopie ;
3. à défaut, sous Windows uniquement, les réglages en base de registre d'avant
   la 10.8.

Une exception : si un ini situé à côté de l'exécutable déclare déjà la
`majeure.mineure` courante dans sa section `[Version]`, VPX crée le dossier de
préférences et s'arrête là, laissant la disposition héritée intacte.
