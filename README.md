# DC_Motor_Project_NucleoSTM32
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
Pour cela, il faut paramétrer la liaison UART de la carte STM32-G431RB. Lors de l'envoie de donnée par l'interface UART, nous ne voyons pas ce que nous envoyons. C'est pourquoi il faut réaliser une fonction qui renvoie à l'utilisateur des caractéres un par un. 
Le principe est le suivant. Une entrée à la console fait un appel à la fonction HAL_UART_RxCpltCallback() où on met un flag à 1. Puis on met le caractère recu dans une variable de type char et on réautorise les interruptions. 
