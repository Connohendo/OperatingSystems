//Joe's Operating System Kernel
//Kernel.c
//Connor Henderson


void printString(char*);
void printChar(char);
void readString(char*);
void readSector(char*, int, int);
void handdleInterrupt(int,int,int,int);
void readFile(char*,char*,int*);
void terminate();
void executeProgram(char*, int*);
void handleTimerInterrupt(int, int);
//void returnFromTimer(int, int);
void killProcess(int);

int processActive[8];
int processStackPointer[8];
int currentProcess = -1;

void main(){
	//print test
	//printString("hello world");

	//read line test
//	char line[80];
//	printString("Enter a line: ");
//	readString(line);
//	printString(line);

	//readSector test
//	char buffer[512];
//	readSector(buffer, 30);
//	printString(buffer);

//	char line[80];
//	makeInterrupt21();
//	interrupt(0x21,1,line,0,0);
//	interrupt(0x21,0,line,0,0);

//	char buffer[13312];   /*this is the maximum size of a file*/
//	int sectorsRead;
//	makeInterrupt21(); 
//	interrupt(0x21, 3, "messag", buffer, &sectorsRead);   /*read the file into buffer*/ 
//	if (sectorsRead>0)
//		interrupt(0x21, 0, buffer, 0, 0);   /*print out the file*/ 
//	else
//		interrupt(0x21, 0, "messag not found\r\n", 0, 0);  /*no sectors read? then print an error*/
//
//	while(1);   /*hang up*/ 
	int i = 0;
	int j = 0;
	for(i; i < 8; i++){
		processActive[i] = 0;
	}
	for(j; j < 8; j++){
		processStackPointer[i] = 0xff00;
	}
//	currentProcess = -1;

	makeInterrupt21();
//	makeTimerInterrupt();
	interrupt(0x21, 4, "shell", 0,0);
	makeTimerInterrupt();
	while(1);
}

void printChar(char letter) {
	// interrupt call with parameters interrupt number, AX, BX, CX, DX respective
	interrupt(0x10,0xe*256+letter, 0, 0, 0);

}

void printString(char* letters) {
	char al = '\n';

	while(*letters != '\0') {
		al = *letters;
		interrupt(0x10,0x0e*256+al,0,0,0);
		letters++;
	}
}

void readString(char *string) {
int i = 0;
int length = 80;
int endStr = 0x0;
int linefeed = 0xa;
int backspace = 0x8;
int enter = 0xd;
while (i != length) {
	string[i] = interrupt(0x16, 0, 0, 0, 0);

	if(i == 80) {
		string[i] = linefeed;
		string[i+1] = endStr;
		break;
		}

	else if(string[i] == enter) {
		string[i] = linefeed;
		string[i+1] = endStr;
		break;
		}
//	else if(string[i] == backspace) {
	//	i--;
	//	string[i] == 
//		}
	i++;
	}
}

void readSector(char* buffer, int sector){
	interrupt(0x13, 2*256+1, buffer, 0*256+sector+1, 0*256+0x80);
}

void handleInterrupt21(int ax, int bx, int cx, int dx){
	if(ax == 0){
	printString(bx);
	}
	else if(ax == 1){
	printString("Enter Line: ");
	readString(bx);
	}
	else if(ax == 2){
	readSector(bx,cx,dx);
	}
	else if (ax == 3){
	readFile(bx,cx);
	}
	else if (ax == 4){
	executeProgram(bx,cx);
	}
	else if (ax == 5){
	terminate();
	}
	else if(ax == 6){
	killProcess(bx);
	}
	else if(ax>7){
	printString("An error occured when ax is greater than 6");
	}
}

void readFile(char* fileName, char* buffer, int* sectorsRead){

	char dir [512];
	int fileEntry;
	int sector;
	int foundFile;
	int entry;
	int i;

	*sectorsRead = 0;
	//handleInterrupt21(3, *fileName, *buffer, *sectorsRead);
	readSector(dir,2);
	for(entry=0; entry < 512; entry+=32){
		foundFile=1;
		for(fileEntry = 0; fileEntry < 6; fileEntry++){
		//printChar(dir[fileEntry+entry]);
		//printChar(fileName[fileEntry]);
		if(fileName[fileEntry]!=dir[fileEntry+entry]){
			foundFile=0;
			//entry+=6
			//printChar(fileEntry);
			break;
		}
		}

	if(foundFile==1){
		break;
	}

}
//handle match 0 
//handle match 1

	if(foundFile==0){
		*sectorsRead=0;
		return;
	}

	for(i=6; i<26; i++){

		sector=dir[entry+i];
		if(sector==0){
			break;
		}
		readSector(buffer,sector);
		buffer=buffer+512;
		*sectorsRead=*sectorsRead+1;
	}
}

void executeProgram(char* name,int* pid){
	char buffer[13312];
	int i;
	int seg;
	int x;
	int dataseg;
	int sectorsRead;

	//read file
	readFile(name,buffer,&sectorsRead);

	if(sectorsRead==0){
		return;
	}
	//check for empty spot
	dataseg = setKernelDataSegment();
	for(x = 0; x < 8; x++){
		if(processActive[x]==0){
			break;
		}
	}
	restoreDataSegment(dataseg);

	if(x==8){
		return;
	}

	//set
	seg=(x+2)*0x1000;

	for(i = 0; i < 512*sectorsRead; i++){
		putInMemory(seg, i, buffer[i]);
	}

	//start the program
	initializeProgram(seg);

	if(pid!=0){
		*pid=x;
	}

	//set active amd update map
	dataseg = setKernelDataSegment();
	processActive[x] = 1;
	processStackPointer[x] = 0xff00;
	restoreDataSegment(dataseg);

}

void terminate(){
	int dataseg;
//	char shellname[6];
//	shellname[0] = 's';
//	shellname[1] = 'h';
//	shellname[2] = 'e';
//	shellname[3] = 'l';
//	shellname[4] = 'l';
//	shellname[5] = '\0';
//	executeProgram(shellname);


	dataseg=setKernelDataSegment();
	processActive[currentProcess] = 0;
	restoreDataSegment(dataseg);
	while(1);

}

void handleTimerInterrupt(int segment, int sp){
	int i;
	int x;
	int dataseg;

//	printChar('T');
//	printChar('I');
//	printChar('C');
//	printChar('\0');

	dataseg = setKernelDataSegment();

	for(i = 0; i < 8; i++)
        {
                putInMemory(0xb800,60*2+i*4,i+0x30);
                if(processActive[i]==1)
                        putInMemory(0xb800,60*2+i*4+1,0x20);
                else
                        putInMemory(0xb800,60*2+i*4+1,0);
        }


	if(currentProcess != -1){
		processStackPointer[currentProcess] = sp;
	}


	x=currentProcess;
	while(1){
		x = x + 1;
		if(x == 8){
			x = 0;
		}
		if(processActive[x] == 1){
			break;
		}
	}

	//update values
	sp = processStackPointer[x];
	currentProcess = x;
	segment = (x+2)*0x1000;


	restoreDataSegment(dataseg);
	returnFromTimer(segment,sp);
}

void killProcess(int pid){
	int dataseg;

//	pid = pid - 1;
//	printChar('p');
//	printChar(pid);

	dataseg=setKernelDataSegment();
	processActive[pid] = 0;
//	processStackPointer[pid] = 0;
	restoreDataSegment(dataseg);

}
