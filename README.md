# GazSecurity – Interlock de sécurité moteur RC (planeur ou avion)

Sécurité matérielle pilotée par un canal dédié (CH6) : le moteur ne peut tourner que si l'interrupteur de sécurité est activé **et** que les gaz sont au minimum.
Pour radio Protronik Ptr6a v2 et récepteur R8x

## Fonctionnalités

- **Coupure moteur forcée** tant que la sécurité (CH6) n'est pas activée : la sortie ESC reste figée à 980 µs (sous le seuil de démarrage), quel que soit ce que fait le manche des gaz.
- **Zone morte** sur le canal gaz (CH3) : en dessous de 1050 µs, la sortie reste forcée à 980 µs, pour éviter tout démarrage intempestif près du zéro.
- **Armement conditionné** : le passage en mode armé n'est autorisé que si l'interrupteur de sécurité est activé **et** que les gaz sont au minimum (≤ 1080 µs) au moment de la bascule — impossible d'armer manche poussé.
- **Recopie transparente du signal gaz** vers l'ESC une fois armé.
- **Détection de perte de signal** : si CH3 ou CH6 ne reçoit plus d'impulsions valides depuis 150 ms, le système repasse automatiquement en mode verrouillé.


## Matériel nécessaire

- 1x ATtiny85 + 2 résistances de 10kohms
- 1x Arduino Uno (ou équivalent) pour flasher l'ATtiny en ISP
- Récepteur RC avec au moins 2 voies libres (gaz + un canal interrupteur pour la sécurité)
- ESC compatible PWM standard (1000–2000 µs)

## Câblage

| Signal              | Broche ATtiny85 (physique) | Broche ATtiny85 (logique) | 
|---------------------|:---------------------------:|:--------------------------:|
| Sortie vers l'ESC    | 6                           | PB1                        |
| Entrée CH3 (gaz)     | 7                           | PB2                        |
| Entrée CH6 (sécurité)| 2                           | PB3                        |
| GND                 | 4                           | —                           |
| VCC (5V)            | 8                           | —                           |
| PULL-UP 10kohms     | 1                           | PB5 (RESET)                 |

<img width="792" height="429" alt="image" src="https://github.com/user-attachments/assets/00689fd2-e8fa-4e0c-b6da-dba59542c054" />

> Le récepteur, l'ESC et l'ATtiny85 doivent partager une **masse commune**. L'ATtiny se place en série sur le fil de signal entre récepteur et ESC — il ne doit pas y avoir de connexion directe résiduelle entre les deux une fois l'ATtiny inséré.

## Flashage

1. Flasher un Uno avec le croquis exemple **ArduinoISP** (fourni avec l'IDE Arduino).
2. Câbler l'Uno en programmateur ISP vers l'ATtiny85 (MOSI/MISO/SCK/RESET + alimentation + condensateur ~10 µF entre RESET et GND de l'Uno).
3. Installer **ATTinyCore** (SpenceKonde) via le gestionnaire de cartes de l'IDE Arduino.
4. Dans l'IDE : Type de carte = *ATtiny25/45/85*, Processeur = *ATtiny85*, Horloge = *8 MHz (interne)*, Programmateur = *Arduino as ISP*.
5. Outils > **Graver la séquence d'initialisation** (règle les fuses, notamment la division d'horloge — étape indispensable, à faire au moins une fois).
6. Croquis > **Téléverser avec un programmateur** (pas le bouton "Téléverser" classique, qui ne fonctionne pas en ISP).

<img width="480" height="446" alt="image" src="https://github.com/user-attachments/assets/2be6973c-4c62-4834-9572-b72a69853807" />

## Configuration

Les seuils sont définis en haut du fichier et peuvent être ajustés selon ton matériel :

| Constante               | Valeur par défaut | Rôle                                                              |
|--------------------------|:------------------:|---------------------------------------------------------------------|
| `MOTOR_OFF_US`           | 980 µs             | Signal forcé quand le moteur doit être coupé                       |
| `THROTTLE_DEADBAND_US`   | 1050 µs            | Sous ce seuil, la sortie reste forcée à `MOTOR_OFF_US`              |
| `THROTTLE_MIN_ARM_US`    | 1080 µs            | Seuil max des gaz autorisant l'armement                            |
| `CH6_UNLOCK_THRESHOLD`   | 1500 µs            | Seuil au-dessus duquel CH6 est considéré comme "déverrouillé"       |

Vérifie ces valeurs avec ta propre radio/récepteur avant utilisation (plage réelle des impulsions, position de repos de l'interrupteur, etc.).

## ⚠️ Avertissement sécurité

Ce firmware pilote directement un moteur capable de blesser. Avant tout vol :

- **Teste toujours hélice démontée** en premier lieu.
- Ce projet est fourni tel quel, sans garantie. Utilisation à tes propres risques.

## Licence

GNU
