# Deplieur-C
Unfolds a 3d model into a multipage PDF pattern to build it with paper/cardboard

Uses Cairo and math libraries, so I compile (gcc) with -lcairo -lm

## Principe du dépliage d'un volume
Le dépliage d'un volume c'est l'ensemble des étapes qui permettent d'obtenir à partir d'un volume, un gabarit 2d permettant de le reproduire.

Le volume est défini comme un polyèdre (forme géométrique formée de surfaces planes polygonales réunies par leurs arêtes).
Pour stocker un tel volume, un fichier 3d va comporter (au minimum) toutes les données nécessaires à la définition de ce polyèdre, c'est-à-dire une liste de polygones.

Il existe plusieurs formats de fichiers 3d, le plus répandu est le format .stl, je lui ai cependant préféré un format moins répandu, le .obj, qui est plus avantageux pour le dépliage en ceci qu'il contient directement une liste de points et une liste polygones exprimées en fonction de cette liste de points, alors que le format .stl lui ne contient qu'une liste de polygones exprimée en points, ce qui nécessite un traitement pour indexer la liste de polygones en fonction des points, ce qui de toutes façone en C ne prendrait pas beaucoup de temps.

La première étape du dépliage d'un volume est le chargement du fichier 3d pour en extraire les points et les polygones.
Pour simplifier les calculs, j'ai préféré trianguler complètement le modèle, c'est-à-dire faire en sorte que les polygones soient tous des triangles.

Ensuite, vient le passage de la 3d à la 2d. Pour cela j'utilise la formule décrite [ici](https://stackoverflow.com/a/8051489)

Puis, on calcule les voisinages à partir de la liste définissant les polygones (faces) :
par exemple, la face 13 qui est définie par les numéros de points (44, 120, 121) aura 3 voisins,
le premier ayant dans sa définition (120, 44),
le second ayant dans sa définition (121, 120)
et le dernier ayant dans sa définition (44, 121)

On peut alors calculer pour chaque paire d'arêtes voisines leur coplanéité (càd si les faces sont coplanaires ou si elles doivent marquer un pli Montagne , coplanéité > 0, ou Vallée, coplanéité < 0).

Puis vient le dépliage proprement dit qui consiste en posant une première face ( par défaut la première), puis essayant d'y lier ses voisines, puis leurs voisines et ainsi de suite jusqu'à ce qu'il n'y ait plus de place sur la page ou qu'il n'y ait plus de faces à lier. Tant que toutes les faces ne sont pas posées, l'on continue sur une nouvelle page avec une nouvelle première face etc.

L'étape de liaison d'une face voisine nécessite de calculer un nouvel emplacement pour son 3e point et la vérification que la face ne vient pas avec ses nouvelles coordonnées empiéter sur ce qui est déjà posé et ne fait pas dépasser la pièce des limites de la page. Si l'on veut ajouter une languette il faut faire les mêmes vérifications pour elle.
