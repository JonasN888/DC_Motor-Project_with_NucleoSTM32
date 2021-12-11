/**
 * \file fonction.c
 * \brief Fonctions pour gérer le shell et la vitesse du moteur
 * \author Jonas N. and Jean Baptiste L.
 * \version 0.3
 * \date 20 november 2021
 *
 */
#include <fonction.h>
#include <string.h>
#include <stdlib.h>


const uint8_t help[] = "Enter <<pinout>>, <<power_on>> or <<power_off>>"; //contient le message d'aide
const uint8_t pinout[] = "PFO-RCC_OSC_IN, PF1-RCC_OSCOUT, PA2-USART2_TX, PA3-USART2_RX, PA14-SYS_JTCK_SWCLK, PA13-JTMS_SWDIO"; //contient la liste des pins utilisés
const uint8_t power_on[] = "turn on motor"; //contient le message d'allumage du moteur
const uint8_t power_off[] = "turn off motor"; //contient le message d'extinction du moteur
const uint8_t not_found[] = "command not found"; //contient le message du commande non reconnue
char enter[] = "\r\n"; //pour avoir un retour à la ligne


/**
 * \fn void gestion_shell(char* ,char , int*, UART_HandleTypeDef )
 * \brief Fonction de création d'une nouvelle instance d'un objet Str_t.
 *
 * \param Chaine de caractère cmd à remplir, le caractère contenant l'écho, pointeur sur l'index du prochain caractère à remplir, huart2
 * \return Ne renvoie rien
 */

void gestion_shell(char* cmd,char uart_rx_echo, int* ptr_idxCmd, UART_HandleTypeDef huart2){
	/*Si le tableau n'est pas déjà rempli, on incrémente l'index
	 * pour mettre la valeur de uart_rx_echo à la position suivante:
	 */
	if((*ptr_idxCmd) < CMD_BUFFER_SIZE){
		cmd[*ptr_idxCmd] = uart_rx_echo;
		(*ptr_idxCmd)++;
		HAL_UART_Transmit(&huart2, &uart_rx_echo, 1, HAL_MAX_DELAY); //on envoie notre chaine de caractère à la console

	}
	//on teste si on a appuyé sur entrée en utilisant le tableau ASCI:
	if(uart_rx_echo==0x0D){
		HAL_UART_Transmit(&huart2, &enter, sizeof(enter), HAL_MAX_DELAY);
		/*On utilise la commande strcnmp pour comparer aux commandes possibles.
		strcmp renvoie 0 lorsque les deux chaines de caractères sont égales:*/

		if(strncmp(cmd,"speed = ",8)==0){
			motor_speed(cmd, huart2);
		}

		else if(strncmp(cmd,"start",5)==0){
			 //demarrer le hacheur 4 quadrants en envoyant un pulse d'une durée de 1 ms:
			 fault_reset_command();
		}

		else if(strncmp(cmd,"help",4)==0){
			//on affiche le contenu de help à la console:
			HAL_UART_Transmit(&huart2, help, sizeof(help), HAL_MAX_DELAY);
		}

		else if(strncmp(cmd,"pinout",6)==0){
			//on affiche le contenu de pinout à la console:
			HAL_UART_Transmit(&huart2, pinout, sizeof(pinout), HAL_MAX_DELAY);
		}

		else if(strncmp(cmd,"power_on",8)==0){
			//on affiche le contenu de power_on à la console:
			HAL_UART_Transmit(&huart2, power_on, sizeof(power_on), HAL_MAX_DELAY);
		}

		else if(strncmp(cmd,"power_off",9)==0){
			//on affiche le contenu de power_off à la console:
			HAL_UART_Transmit(&huart2, power_off, sizeof(power_off), HAL_MAX_DELAY);
		}

		else{
			//on affiche le contenu de not_found à la console:
			HAL_UART_Transmit(&huart2, not_found, sizeof(not_found), HAL_MAX_DELAY);
		}
		//on vide la chaine de caractère en mettant à 0 chaque élément de la liste des caractères
		for(int i=0;i<CMD_BUFFER_SIZE;i++){
		    cmd[i] = "\0";
		}
		//on met l'index pointant vers le prochain caractère à remplir à 0:
		*ptr_idxCmd=0;
		HAL_UART_Transmit(&huart2, enter, sizeof(enter), HAL_MAX_DELAY);
	}
}

/*On a a fait le test avec plusieurs rapports cycliques:
À 70% et 100% on a une erreur pour le hacheur "SHUNT OVERCURRENT" et le moteur ne démarre pas
à 60% le moteur tourne lentement
à 55% le moteur ne démarre pas à cause des frottements secs
*/

/**
 * \fn void fault_reset_command(void)
 * \brief Démarrer le hacheur
 *
 * \param Pas de paramètres. On met crée un signal carée pendant une durée de 1ms (>2us)
 * \return Ne renvoie rien
 */
void fault_reset_command(void){
	HAL_GPIO_WritePin(Pulse_GPIO_Port, Pulse_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(Pulse_GPIO_Port, Pulse_Pin, GPIO_PIN_SET); //on met la sortie à 5 Volts
	HAL_Delay(1);
	HAL_GPIO_WritePin(Pulse_GPIO_Port, Pulse_Pin, GPIO_PIN_RESET); //on met la sortie à 0 Volts après 1ms
}

/**
 * \fn void motor_speed(char* cmd, UART_HandleTypeDef huart2)
 * \brief Changer la vitesse du moteur
 *
 * \param Le buffer cmd pour récupérer la vitesse, le handler huart2 pour pouvoir débuguer le code sur la liasion série
 * \return Ne renvoie rien, car on veut seulement modifier les registres pour changer le rapport cyclique
 */
void motor_speed(char* cmd, UART_HandleTypeDef huart2){
	char speed[3]; //buffer pour récupérer la valeur pour la vitesse. On choisit de prendre 3 digits correspondant au rapport cyclique
	speed[0] = cmd[8];
	speed[1] = cmd[9];
	speed[2] = cmd[10];
	double alpha = atoi(speed);
	if (alpha > 100){ // on teste si notre valeur désirée est plus grande que la vitessse maximale
		alpha = 100;
	}
	if (alpha < 0){ // on teste si notre valeur désirée est plus grande en valeur absolue que la vitessse maximale
		alpha = 0;
	}
	alpha = alpha/100;
	//HAL_UART_Transmit(&huart2, speed, sizeof(speed), HAL_MAX_DELAY); //ligne de code pour débuguer

	//on modifie les registres CCR1 et CCR2 pour mettre à jour le rapport cyclique:
	TIM1->CCR1 = (int)(alpha*1024);
	TIM1->CCR2 = (int)((1-alpha)*1024);
}

