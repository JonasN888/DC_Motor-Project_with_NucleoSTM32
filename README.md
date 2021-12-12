# DC_Motor_Project_with_Nucleo_STM32
Projet pour commander une MCC avec le Nucleo Board STM32


![IMG_20211116_105229](https://user-images.githubusercontent.com/95878275/145672435-6eff55a5-bf14-4104-b1ba-ca9f1e4762ed.jpg)

L'objectif de ce projet est de commander un moteur de type MCC et de l'asservir en utilisant une carte Nucleo STM32. 

Il y a 4 étapes:

1. Réaliser une interface de type "console" pour commande le hacheur d'un moteur
2. Réaliser la commande des 4 transistors du hacheur en commande complémentaire décalée
3. Faire l'acquisition des différents capteurs
4. Réaliser l'asservissement en temps réel



CONSOLE UART:

Via le port USB de la carte STM32G431, nous souhaitons avoir une interface permettant d'avoir des informations sur l'état du moteur et l'envoie de commandes. Pour cela, 
il faut paramétrer la liaison UART de la carte STM32-G431RB. Lors de l'envoie des données par l'interface UART, nous ne voyons pas ce que nous envoyons. C'est pourquoi il faut réaliser une fonction qui renvoie à l'utilisateur des caractéres un par un. 
Le principe est le suivant: On autorise les interruptions venant du UART. Une entrée à la console va faire appel à la fonction HAL_UART_RxCpltCallback() où on met le caractère reçu dans une variable de type char et on reautorise les interruptions. Dans la boucle while() du main, on teste s'il y avait une interruption en utilisant un flag qui se met à 1 quand la fonction HAL_UART_RxCpltCallback() est appelée. On appelle ensuite notre fonction "gestion_shell()" qui affiche le caractère sur la console. Ensuite, il faut remettre notre flag à 0.
Pour pouvoir utiliser des commmandes dans notre shell, on stocke chaque caractère reçu dans un tableau "cmd" de type char en incrémentant un pointeur après chaque nouveau caractère. Afin de comparer les chaînes de caractère aux commandes (start, speed, help, pinout...) on utilise la fonction strncmp(). L'appui sur la touche Entrée peut être détectée en utilisant le tableau d'ASCI en comparant notre caractère à 0x0D.


COMMANDE DE 4 TRANSISTORS DU HACHEUR:

On génère 4 PWM en commande décalé. Pour cela, on utilise le Timer 1 sur les chaînes 1 et 2 en mode "PWM Generation CH1 CH1N". Afin d'avoir une fréquence de 16kHz pour faire tourner proprement la MCC, on prend PSC = 5-1 et ARR = 1024-1. Le mode de comptage est le "center aligend mode". On visualise les 4 signaux sur l'oscilloscope afin de vérifer le bon fonctionnement ainsi que la fréquence de nos signaux:

![IMG_9188](https://user-images.githubusercontent.com/95878275/145674111-7a581ea2-4b7c-4aa9-9c5c-54700eaf9e32.PNG)

Le temps mort doit être supérieur à 2us. On peut réaliser ce temps mort en réglant le registre "DeadTime" à 203.

Le câblage de la carte STM32 au hacheur se fait en utilisant la documentation "PowerModule_70097A.pdf".

![IMG_20211116_105327](https://user-images.githubusercontent.com/95878275/145674224-aaccca4f-5367-4efa-bddf-2fe2d45ffd34.jpg)


![IMG_20211109_092212](https://user-images.githubusercontent.com/95878275/145674236-b6eb5d1b-f74c-4702-8326-4da7fae88389.jpg)

Afin de démarrer correctement le hacheur, il faut envoyer un signal carrée pendant au minimum 2us. On peut créér ce signal carrée dans une fonction start() qu'on appelle dans la fonction gestion_shell() si le message envoyé correspond bien à la chaîne de caractère "start".

On peut changer directement le rapport cyclique du hacheur en utilisant des structures de type TIM1->CCR1. On constate que le moteur ne démarre pas encore pour alpha = 0,55 à cause de frottements secs. À partir de alpha = 0.6, le moteur commence à tourner lentement. Il est conseillé de ne pas augmenter la vitesse trop rapidement, sinon le moteur demande un courant qui dépasse la valeur maximale autorisée par le hacheur. Un rapport cyclique inférieur à 0.45 fait tourner le moteur dans l'autre sens.

Il est pratique de régler la vitesse du moteur directement dans une fonction. On appelle cette fonction lorsque notre chaîne de caractère entrée à la console correspond bien à "speed". Dans notre cas, un rapport cyclique de 1 correspond à la vitesse maximale du moteur. On récupère la vitesse via notre tableu "cmd" après avoir fait une conversion en entier. On élimine les cas où le rapport cyclique prend une valeur non autorisée.



AQUISITION DES CAPTEURS:


La prochaine étape est de faire l'acquisition des données venant du capteur Hall pour mesurer le courant. On active le ADC ainsi que le DMA dans le fichier.ioc en autorisant les interruptions venant du DMA. On peut ensuite tester le capteur en affichant les données à la console. La documentation PowerModule_70097A.pdf nous dit que la conversion se fait de la manière suivante: 12,5A/V avec 2,5V = 0A. Cela  correspond à l'équation suivante qu'on utilise pour calculer le courant en ampères: I = 12,5 * U - 30.

Pour mesurer la vitessse du moteur, on peut utiliser l'encodeur déjà inclu dans la MCC. On configure le Timer2 en "counter mode" ce qui nous permet de compter de manière automatique l'evolution du moteur qui sera stocké dans le registre CNT du Timer2. 
Afin de mesurer la vitesse, on peut activer un autre Timer qui va générer une interruption toutes les 1ms. Dans cette interruption (HAL_TIM_PeriodElapsedCallback()) on vérifie d'abord si l'interruption vient du Timer correspondant. On peut ensuite calcuer l'écart entre la position actuelle et la position précédente, puis multiplier le résultat par la fréquence du Timer pour avoir la vitesse en tours par seconde. 


Notre moteur a des caractéristiques particulières qu'il faut déterminer. 
On trouve les caractéristiques suivantes: R = 1,5 Ohm, tau = 12,4ms, L = 0,0186 H, kphi = 0,132 Vs, f=0,0011 Nms et Gamma_0 = 0,2506 Nm.

Ici de manière exemplaire pour le temps de montée:

![IMG_20211130_095644](https://user-images.githubusercontent.com/95878275/145679855-ee7cc0e3-b5aa-4986-8a73-71359b0095f8.jpg)

![IMG_20211130_103622](https://user-images.githubusercontent.com/95878275/145679870-0c087f3f-a28d-4fd8-ac21-ae342b701ca8.jpg)

