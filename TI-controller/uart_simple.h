/*ConfigUART
* Sets up the UART interface via USCI
* INPUT: None
* RETURN: None
*/
void ConfigUART(void);

/*Send_Char
* Sends a char to the UART. Will wait if the UART is busy
* INPUT: Char to send
* RETURN: None
*/
void Send_Char(char chr);

/*Print_UART
* Sends a string to the UART. Will wait if the UART is busy
* INPUT: Pointer to String to send
* RETURN: None
*/
void Print_UART(char *ch);