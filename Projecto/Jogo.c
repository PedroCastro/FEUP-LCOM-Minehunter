#include <minix/syslib.h>
#include <minix/drivers.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ModGrafico.h"
#include "Jogo.h"
#include "ModRato.h"
#include "ModTeclado.h"
#include "ModTimer.h"
#include "bitmap.h"


Bitmap* bitmap_table;
Bitmap* fundo;

int jogo(int difficulty) {
	sef_startup();
	bitmap_table = loadBitmap("home/lcom/Projecto/res/images/Tabela_Expert.bmp");
	fundo = loadBitmap("home/lcom/Projecto/res/images/Fundo.bmp");
	global_counter = 0;
	buffer = malloc(videoMemSize * BITS_PER_PIXEL	/8);
	bufferRato =malloc(videoMemSize * BITS_PER_PIXEL	/8);
	int irq_set_timer,irq_set_keyboard, irq_set_mouse;
	int contador = 0,breaker = 1,two_bytes = 0, mouse_byte;
	int ipc_status, loops = 0, teste = 0;
	message msg;
	create_interrupts(&irq_set_timer,&irq_set_keyboard,&irq_set_mouse);
	int filled = 0;
	Mine** table = create_table(difficulty);
	video_mem = vg_init(0x117);
	drawBitmap(fundo,0,0,ALIGN_LEFT,buffer);
	drawBitmap(bitmap_table,0,0,ALIGN_LEFT,buffer);
	while (breaker) {
		if (driver_receive(ANY, &msg, &ipc_status) != 0) {
			printf("driver_receive failed with: %d");
			continue;
		}
		//printf("msg.notifyArg: 0x%X\n", msg.NOTIFY_ARG);
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set_timer) {
					timer_int_handler();
					if(global_counter % 1 == 0)
					{
						update_screen();
					}

					if (global_counter == 60) {
						loops++;
						global_counter = 0;
					}
				}
				if (msg.NOTIFY_ARG & irq_set_keyboard)
				{
					kbd_int_handler();
					if(scan_code==TWO_BYTE_CODE){two_bytes = 1;}
					else if(scan_code>>7){
						if(two_bytes) ;
						else ;
						if (scan_code==BREAK_CODE_ESC)
							breaker = 0;
					}
					else
					{
						if (two_bytes){
							two_bytes=0;
						}
						else ;
					}
				}
				if (msg.NOTIFY_ARG & irq_set_mouse)
				{
					rato = getRato();
					mouse_byte = mouse_int_handler();
					if (contador == 0) {
						if (!first_byte(mouse_byte)) {
							contador = 0;
							continue;
						}
					}
					rato->packets[contador] = mouse_byte;
					if (contador == 2) {
						contador = 0;
						{
							updateMouse();
							teste = 1;
						}
						continue;
					}
					contador++;
					if(rato->packets[0]&BIT(0) && rato->leftButtonDown == 0)
					{
						rato->leftButtonDown = 1;
						rato->leftButtonReleased = 0;
						click_screen(&table,rato->x,rato->y,difficulty,&filled);
					}
					else if(!(rato->packets[0]&BIT(0)))
					{
						rato->leftButtonDown = 0;
						rato->leftButtonReleased = 1;
					}
					if(rato->packets[0]&BIT(1) && rato->rightButtonDown == 0)
					{
						rato->rightButtonDown = 1;
						rato->rightButtonReleased = 0;
						right_click_screen(&table,rato->x,rato->y,difficulty);
					}
					else if(!(rato->packets[0]&BIT(1)))
					{
						rato->rightButtonDown = 0;
						rato->rightButtonReleased = 1;
					}
				}
				break;
			}
		}

	}
	vg_exit();
	stop_interrupts();
	free(rato);
	free(buffer);
	free(bufferRato);
	free(table);

}


Mine** fill_table(Mine** table,int difficulty,int k_mouse, int j_mouse)
{
	if(difficulty == 2){
		int i;
		time_t t;
		srand((unsigned) time(&t));
		for(i = 0; i < NUM_MINES_EXPERT;i++)
		{

			int j = rand() % HEIGHT_EXPERT;
			int k = rand() % WIDTH_EXPERT;
			if(table[k][j].valor == -1)
			{i--;}
			else if(table[k][j].valor >= 8)
			{i--;}
			else if((k == k_mouse || k == k_mouse + 1 || k == k_mouse - 1)&&(j == j_mouse || j ==j_mouse -1 || j == j_mouse + 1))
			{
				i--;
			}
			else{
				table[k][j].valor = -1;
				if(k != 0) // parede lateral esquerda /// certo!!
					if(table[k-1][j].valor != -1)
						table[k-1][j].valor += 1;
				if(k != WIDTH_EXPERT - 1) /// parede laterral direita/// certo!!
					if(table[k+1][j].valor != -1)
						table[k+1][j].valor += 1;
				if(j != 0) /// parede vertical de cima /// certo!!
					if(table[k][j - 1].valor != -1)
						table[k][j - 1].valor += 1;
				if(j != HEIGHT_EXPERT - 1)///parede vertical de baixo/// certo!!
					if(table[k][j+1].valor != -1)
						table[k][j + 1].valor += 1;
				if(k != 0 && j != 0) ///canto superior esquerdo ///certo
					if(table[k - 1][j - 1].valor != -1)
						table[k - 1][j - 1].valor += 1;
				if(k != WIDTH_EXPERT - 1 && j != HEIGHT_EXPERT - 1)  /// canto inferior direito /// certo
					if(table[k + 1][j + 1].valor != -1)
						table[k + 1][j + 1].valor += 1;
				if(k != WIDTH_EXPERT - 1 && j != 0)  /// canto superior direito
					if(table[k + 1][j - 1].valor != -1)
						table[k + 1][j - 1].valor += 1;
				if(k != 0 && j != HEIGHT_EXPERT - 1)/// canto inferior esquerdo
					if(table[k - 1][j + 1].valor != -1)
						table[k - 1][j + 1].valor += 1;
			}
		}
	}
		return table;

}


Mine** create_table(int difficulty){
	if(difficulty == 2) ///expert
	{
		int i;
		Mine** table;
		table = malloc(WIDTH_EXPERT *sizeof(Mine *));
		for (i = 0; i < WIDTH_EXPERT; i++)
			table[i] = malloc( HEIGHT_EXPERT *sizeof(Mine));
		int j;
		for(i = 0;i < WIDTH_EXPERT;i++)
			for(j = 0; j < HEIGHT_EXPERT;j++)
			{
				table[i][j].valor = 0;
				table[i][j].carregado = 0;
			}
		return table;
	}
}

int click_screen(Mine*** table, int x, int y, int difficulty, int *filled){
	if((*filled) == 0)
	{
		printf("cria nova table\n");
		*(table) = fill_table((*table),difficulty,(x-32)/32,(y-186)/32);
		*(filled)  = 1;
	}
	printf("continua para o jogo\n");
	Bitmap* quadrado;
	if(difficulty == 2){
		if(x-32 > 0 && x < 992)
		{
			if(y-186>0 && y < 698)
			{
				if((*table)[(x-32)/32][(y-186)/32].carregado == 0){
					(*table)[(x-32)/32][(y-186)/32].carregado = 1;
					int valor = (*table)[(x-32)/32][(y-186)/32].valor;
					if(valor == 0)
						click_vazio(table, ((x-32)/32), ((y-186)/32), difficulty);
					if(valor == 0)
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Vazio.bmp");
					else if(valor == -1)
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Bomba.bmp");
					else if(valor == 1)
					{
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado1.bmp");
					}
					else if(valor == 2)
					{
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado2.bmp");
					}
					else if(valor == 3)
					{
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado3.bmp");
					}
					else if(valor == 4)
					{
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado4.bmp");
					}
					else if(valor == 5)
					{
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado5.bmp");
					}
					else if(valor == 6)
					{
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado6.bmp");
					}
					else if(valor == 7)
					{
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado7.bmp");
					}
					else
					{
						quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado8.bmp");
					}
					drawBitmap(quadrado,(((x-32)/32)*32)+32,(((y-186)/32)*32)+186,ALIGN_LEFT,buffer);
				}
			}
		}
	}
}


int right_click_screen(Mine*** table, int x, int y, int difficulty){
	Bitmap* quadrado;
	if(difficulty == 2){
		if(x-32 > 0 && x < 992)
		{
			if(y-186>0 && y < 698)
			{
				if((*table)[(x-32)/32][(y-186)/32].carregado == 0){
					(*table)[(x-32)/32][(y-186)/32].carregado = 2;
					quadrado = loadBitmap("home/lcom/Projecto/res/images/Bandeira.bmp");
					drawBitmap(quadrado,(((x-32)/32)*32)+32,(((y-186)/32)*32)+186,ALIGN_LEFT,buffer);
				}
				else if((*table)[(x-32)/32][(y-186)/32].carregado == 2)
				{
					(*table)[(x-32)/32][(y-186)/32].carregado = 0;
					quadrado = loadBitmap("home/lcom/Projecto/res/images/Quadrado_Tapado.bmp");
					drawBitmap(quadrado,(((x-32)/32)*32)+32,(((y-186)/32)*32)+186,ALIGN_LEFT,buffer);
				}

			}
		}
	}

}

void click_vazio(Mine*** table, int k, int j, int difficulty)
{
	int filled = 1;
	if(difficulty == 2){
		if(k != 0)
			if((*table)[k-1][j].valor != -1){
				if((*table)[k-1][j].carregado == 0){
					click_screen(table, (k-1)*32+32+2, (j)*32 + 186+2, difficulty,&filled);
				}
			}
		if(k != WIDTH_EXPERT - 1)
			if((*table)[k+1][j].valor != -1){
				if((*table)[k+1][j].carregado == 0){
					click_screen(table, (k+1)*32+32+2, (j)*32 + 186+2, difficulty,&filled);
				}
			}
		if(j != 0)
			if((*table)[k][j - 1].valor != -1){
				if((*table)[k][j-1].carregado == 0){
					click_screen(table, (k)*32+32+2, (j-1)*32 + 186+2, difficulty,&filled);
				}
			}
		if(j != HEIGHT_EXPERT - 1)
			if((*table)[k][j+1].valor != -1){
				if((*table)[k][j+1].carregado == 0){
					click_screen(table, (k)*32+32+2, (j+1)*32 + 186+2, difficulty,&filled);
				}
			}
		if(k != 0 && j != 0)
			if((*table)[k - 1][j - 1].valor != -1){
				if((*table)[k-1][j-1].carregado == 0){
					click_screen(table, (k-1)*32+32+2, (j-1)*32 + 186+2, difficulty,&filled);
				}
			}
		if(k != WIDTH_EXPERT - 1 && j != HEIGHT_EXPERT - 1)
			if((*table)[k + 1][j + 1].valor != -1){
				if((*table)[k+1][j+1].carregado == 0){
					click_screen(table, (k+1)*32+32+2, (j+1)*32 + 186+2, difficulty,&filled);
				}
			}
		if(k != WIDTH_EXPERT - 1 && j != 0)
			if((*table)[k + 1][j - 1].valor != -1){
				if((*table)[k+1][j-1].carregado == 0){
					click_screen(table, (k+1)*32+32+2, (j-1)*32 + 186+2, difficulty,&filled);
				}
			}
		if(k != 0 && j != HEIGHT_EXPERT - 1)
			if((*table)[k - 1][j + 1].valor != -1){
				if((*table)[k-1][j+1].carregado == 0){
					click_screen(table, (k-1)*32+32+2, (j+1)*32 + 186+2, difficulty,&filled);
				}
			}
	}
}


	void create_interrupts(int *irq_set_timer,int *irq_set_keyboard,int *irq_set_mouse){
	*irq_set_timer = timer_subscribe_int();
	*irq_set_keyboard = kbd_subscribe_int();
	*irq_set_mouse = mouse_subscribe_int();
	enable_packets();
}

void stop_interrupts(){
	mouse_unsubscribe_int();
	kbd_unsubscribe_int();
	timer_unsubscribe_int();
}
