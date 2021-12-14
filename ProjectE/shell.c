void enableInterrupts();

char command[80];
char buffer[13312];

void main(){

	while(1){
	syscall(0,"SHELL>");
	syscall(1,command);
	if(command[0] == 't' && command[1] == 'y' && command[2] == 'p' && command[3] == 'e'){
		syscall(3,command+5,buffer);
		if(buffer[0] == '\0'){
			syscall(0,"file not found");
		}
		else {
			syscall(0,buffer);
			buffer[0]= '\0';
		}
	}
	else if(command[0] == 'e' && command[1] == 'x' && command[2] == 'e' && command[3] == 'c'){
		syscall(4,command+5);
	}
	else if(command[0] == 'k' && command[1] == 'i' && command[2] == 'l' && command[3] == 'l'){
		syscall(6,command[5]-'0');
	}
	else{
		syscall(0,"Bad command");
	}
	}
}
