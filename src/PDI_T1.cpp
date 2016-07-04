#include <bits/stdc++.h>
#define byte unsigned char
#define b16 unsigned short int
#define b32 unsigned int

using namespace std;

typedef struct{
    unsigned char type[2];                      //2 bytes
    unsigned int size;                          //4 bytes
    unsigned short int reserved1, reserved2;    //4 bytes   14 bytes
    unsigned int offset;                        //4 bytes
} HEADER;

typedef struct{
    unsigned int size;
    unsigned int width, height;
    unsigned short int planes;
    unsigned short int bits;
    unsigned int compression;
    unsigned int imagesize;
    unsigned int xresolution, yresolution;
    unsigned int ncolours;
    unsigned int importancolours;
} INFOHEADER;

typedef struct{
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
}RGBSQUAD;

typedef struct{
    unsigned char B;
    unsigned char G;
    unsigned char R;
}PIXEL;

unsigned char **imageS;
PIXEL **image;
int *mask;
RGBSQUAD *palet;
int fills, npadding, resto;
int boool, acumx;
char namef[256];

void emascara(unsigned short b){
    if(b==1){
        free(mask);
        mask = (int*) malloc(8*sizeof(PIXEL));
        for(int i=0; i<8; i++)
            mask[i] = 0x1 << i;

    }else if(b==4){
        free(mask);
        mask = (int*) malloc(2*sizeof(PIXEL));
        mask[0] = 15;
        mask[1] = 240;
    }
}

void expandir(unsigned char bits, int y, unsigned short int b){
    boool = 1;
    if(b==1){
        for(int i=7; i>=0; i--){
            if(bits & mask[i]){
                image[y][acumx].B = 0;
                image[y][acumx].G = 0;
                image[y][acumx].R = 0;
            }else{
                image[y][acumx].B = 255;
                image[y][acumx].G = 255;
                image[y][acumx].R = 255;
            }
            acumx++;
        }
    }else if(b==4){
        for(int i=1; i>=0; i--){
            if(!i){
                image[y][acumx].B = palet[bits & mask[i]].rgbBlue;
                image[y][acumx].G = palet[bits & mask[i]].rgbGreen;
                image[y][acumx].R = palet[bits & mask[i]].rgbRed;
            }else{
                image[y][acumx].B = palet[(bits & mask[i])>>4].rgbBlue;
                image[y][acumx].G = palet[(bits & mask[i])>>4].rgbGreen;
                image[y][acumx].R = palet[(bits & mask[i])>>4].rgbRed;
            }
           acumx++;
        }
    }
}

void LoadBMP(HEADER *header, INFOHEADER *info){
    FILE *f;
    unsigned char padding;

    f = fopen(namef, "rb+");
    if(!f){
        printf("No se puede abrir el archivo");
        exit(1);
    }

    fseek(f, 0, SEEK_SET);
    fread(&header->type, sizeof(byte), 2, f);       //2 byte
    fread(&header->size, sizeof(b32), 1, f);        //4 byte
    fread(&header->reserved1, sizeof(b16), 1, f);   //2 byte 14 bytes
    fread(&header->reserved2, sizeof(b16), 1, f);   //2 byte
    fread(&header->offset, sizeof(b32), 1, f);      //4 byte

    fread(&info->size, sizeof(b32), 1, f);              //4 byte
    fread(&info->width, sizeof(b32), 1, f);             //4 byte
    fread(&info->height, sizeof(b32), 1, f);            //4 byte
    fread(&info->planes, sizeof(b16), 1, f);            //2 byte
    fread(&info->bits, sizeof(b16), 1, f);              //2 byte
    fread(&info->compression, sizeof(b32), 1, f);       //4 byte
    fread(&info->imagesize, sizeof(b32), 1, f);         //4 byte
    fread(&info->xresolution, sizeof(b32), 1, f);       //4 byte
    fread(&info->yresolution, sizeof(b32), 1, f);       //4 byte
    fread(&info->ncolours, sizeof(b32), 1, f);          //4 byte
    fread(&info->importancolours, sizeof(b32), 1, f);   //4 byte

    if(header->type[0]!='B' || header->type[1]!='M')
        printf ("La imagen debe tener un forma BMP.\n");


    fills = header->offset - (info->ncolours*sizeof(RGBSQUAD));

    if(info->bits < 24){
        palet = (RGBSQUAD *) malloc(info->ncolours*sizeof(RGBSQUAD));

        fseek(f, fills, SEEK_SET);

        for(int i=0; i<info->ncolours; i++)
            fread(&palet[i], sizeof(RGBSQUAD), 1, f);

        fseek(f, header->offset, SEEK_SET);

        if(info->bits == 8){
            image =(PIXEL **) calloc(info->height, sizeof(PIXEL*));
            imageS = (unsigned char **) malloc(info->height*sizeof(unsigned char*));
            for(int i=0; i<info->height; i++){
                imageS[i] = (unsigned char*) malloc(info->width*sizeof(unsigned char));
                image[i] = (PIXEL *) calloc(info->width, sizeof(PIXEL));
            }

            npadding = (info->width) % 4;
            if(npadding > 0)
                resto = 4-npadding;

            for(int i=info->height-1; i>=0; i--){
                for(int j=0; j<info->width; j++)
                    fread(&imageS[i][j], sizeof(byte), 1, f);

                for(int y=0; y<resto; y++)
                    fread(&padding, sizeof(char), 1, f);
            }

        }else if(info->bits == 4){
            int w=info->width, h=info->height;
            acumx =0;

            emascara(info->bits);

            image =(PIXEL **) calloc(info->height, sizeof(PIXEL*));
            imageS = (unsigned char **) malloc(info->height*sizeof(unsigned char*));
            for(int i=0; i<info->height; i++){
                imageS[i] = (unsigned char*) malloc(info->width*sizeof(unsigned char));
                image[i] = (PIXEL *) calloc(info->width, sizeof(PIXEL));
            }

            if((info->width*info->bits)%32 == 0){
                for(int i=info->height-1; i>=0; i--){
                    acumx = 0;
                    for(int j=0; j<((info->width)/2); j++){
                        fread(&imageS[i][j], sizeof(byte), 1, f);
                        expandir(imageS[i][j], i, info->bits);
                    }
                }
            }else{
                int j;
                int RW = info->width*4;
                int mult = 32*((RW/32)+1);
                int PD = mult - RW;
                int ByteF = RW/8;
                int ByteP = PD/8;

                for(int i=info->height-1; i>=0; i--){
                    acumx = 0;
                    for(j=0; j<((info->width)/2); j++){
                        fread(&imageS[i][j], sizeof(byte), 1, f);
                        expandir(imageS[i][j], i, info->bits);
                    }
                    if((ByteF + ByteP) != ((RW + PD)/8)){
                        fread(&imageS[i][j], sizeof(byte), 1, f);
                        expandir(imageS[i][j], i, info->bits);
                    }

                    for(int y=0; y<ByteP; y++)
                        fread(&padding, sizeof(char), 1, f);
                }
           }
        }else if(info->bits == 1){
            int w=info->width, h=info->height;
            acumx =0;

            emascara(info->bits);

            image =(PIXEL **) calloc(info->height, sizeof(PIXEL*));
            imageS = (unsigned char **) malloc(info->height*sizeof(unsigned char*));
            for(int i=0; i<info->height; i++){
                imageS[i] = (unsigned char*) malloc(info->width*sizeof(unsigned char));
                image[i] = (PIXEL *) calloc(info->width, sizeof(PIXEL));
            }

            if((info->width*info->bits)%32 == 0){
                for(int i=info->height-1; i>=0; i--){
                    acumx = 0;
                    for(int j=0; j<((info->width)/8); j++){
                        fread(&imageS[i][j], sizeof(byte), 1, f);
                        expandir(imageS[i][j], i, info->bits);
                    }
                }
             }else{
                int j;
                int RW = info->width;
                int mult = 32*((RW/32)+1);
                int PD = mult - RW;
                int ByteF = RW/8;
                int ByteP = PD/8;

                for(int i=info->height-1; i>=0; i--){
                    acumx = 0;
                    for(j=0; j<((info->width)/8); j++){
                        fread(&imageS[i][j], sizeof(byte), 1, f);
                        expandir(imageS[i][j], i, info->bits);
                    }
                    if((ByteF + ByteP) != ((RW + PD)/8)){
                        fread(&imageS[i][j], sizeof(byte), 1, f);
                        expandir(imageS[i][j], i, info->bits);
                    }

                    for(int y=0; y<ByteP; y++)
                        fread(&padding, sizeof(char), 1, f);
                }
           }
         }
    }else if(info->bits == 24){
        npadding = (info->width*3) % 4;
        if(npadding > 0)
            resto = 4-npadding;

        image =(PIXEL**) calloc(info->height, sizeof(PIXEL*));
        for(int i=0; i<info->height; i++)
            image[i] = (PIXEL*) calloc(info->width, sizeof(PIXEL));

        fseek(f, header->offset, SEEK_SET);

        for(int i=info->height-1; i>=0; i--){
            for(int j=0; j<info->width; j++){
                fread(&image[i][j], sizeof(byte), 3, f);
            }
            for(int y=0; y<resto; y++)
                fread(&padding, sizeof(char), 1, f);
        }
    }
    fclose(f);
}


void SaveBMP(HEADER *header, INFOHEADER *info){
    FILE *f;
    int padding;

    f = fopen("Saliada.bmp", "wb+");
    if(!f){
        printf("No se pudo crear el archivo de salida");
        exit(1);
    }

    fseek(f, 0, SEEK_SET);
    fwrite(&header->type, sizeof(byte), 2, f);
    fwrite(&header->size, sizeof(b32), 1, f);
    fwrite(&header->reserved1, sizeof(b16), 1, f);
    fwrite(&header->reserved2, sizeof(b16), 1, f);
    fwrite(&header->offset, sizeof(b32), 1, f);

    fwrite(&info->size, sizeof(b32), 1, f);
    fwrite(&info->width, sizeof(b32), 1, f);
    fwrite(&info->height, sizeof(b32), 1, f);
    fwrite(&info->planes, sizeof(b16), 1, f);
    fwrite(&info->bits, sizeof(b16), 1, f);
    fwrite(&info->compression, sizeof(b32), 1, f);
    fwrite(&info->imagesize, sizeof(b32), 1, f);
    fwrite(&info->xresolution, sizeof(b32), 1, f);
    fwrite(&info->yresolution, sizeof(b32), 1, f);
    fwrite(&info->ncolours, sizeof(b32), 1, f);
    fwrite(&info->importancolours, sizeof(b32), 1, f);

    resto =0;

    if(info->bits < 24){
        npadding = (info->width) % 4;
        if(npadding > 0)
            resto = 4-npadding;

       fseek(f, fills, SEEK_SET);

        for(int i=0; i<info->ncolours; i++)
            fwrite(&palet[i], sizeof(byte), 4, f);

        fseek(f, header->offset, SEEK_SET);

        for(int i=info->height-1; i>=0; i--){
            for(int j=0; j<info->width; j++){
                fwrite(&imageS[i][j], sizeof(byte), 1, f);
            }
            if(info->bits == 8){
                for(int y=0; y<resto; y++)
                    fwrite(&padding, sizeof(char), 1, f);
            }
        }

    }else if(info->bits == 24){
        npadding = (info->width*3) % 4;
        if(npadding > 0)
            resto = 4-npadding;

        fseek(f, header->offset, SEEK_SET);

        for(int i=info->height-1; i>=0; i--){
            for(int j=0; j<info->width; j++){
                fwrite(&image[i][j],sizeof(PIXEL),1, f);
            }
        for(int y=0; y<resto; y++)
            fwrite(&padding, sizeof(char), 1, f);
        }

    }
    fclose(f);
}

void negative(HEADER *header, INFOHEADER *info){
    if(info->bits >= 24){
        for(int r=0; r<info->height; r++){
            for(int c=0; c<info->width; c++){
                image[r][c].B = 255 - image[r][c].B;
                image[r][c].G = 255 - image[r][c].G;
                image[r][c].R = 255 - image[r][c].R;
            }
        }
    }else{
        for(int i=0; i<info->ncolours; i++){
            palet[i].rgbBlue = 255 - palet[i].rgbBlue;
            palet[i].rgbGreen = 255 - palet[i].rgbGreen;
            palet[i].rgbRed = 255 - palet[i].rgbRed;
        }
    }
}

void rotatesC(INFOHEADER *info){
    PIXEL **image2;
    unsigned char **imageS2;
    int w, h, t;

    image2 =(PIXEL**) malloc(info->width*sizeof(PIXEL*));
    for(int i=0; i<info->width; i++)
            image2[i] = (PIXEL*) malloc(info->height*sizeof(PIXEL));

    imageS2 = (unsigned char **) calloc(info->width,sizeof(unsigned char*));
    for(int i=0; i<info->width; i++)
            imageS2[i] = (unsigned char*) calloc(info->height,sizeof(unsigned char));

    if(info->bits ==24){
        t=info->height-1;   //horario
        for(int i=0; i<info->height; i++){
            for(int j=0; j<info->width; j++){
                image2[j][t]=image[i][j];
            }
            t--;
        }
    }else if(info->bits == 8){

        t=info->height-1;   //horario
        for(int i=0; i<info->height; i++){
            for(int j=0; j<info->width; j++){
                imageS2[j][t]=imageS[i][j];
            }
            t--;
        }
    }
    w = info->height;
    h = info->width;
    info->height = h;
    info->width = w;
    printf("%d %d\n", h, info->width);
    w = info->xresolution;
    h = info->yresolution;
    info->xresolution = h;
    info->yresolution = w;

    npadding = (info->width*3) % 4;
    image = image2;
    imageS = imageS2;
}

void rotatesCC(INFOHEADER *info){
    PIXEL **image2;
    unsigned char **imageS2;
    int w, h, t;

    image2 =(PIXEL**) malloc(info->width*sizeof(PIXEL*));
    for(int i=0; i<info->width; i++)
            image2[i] = (PIXEL*) malloc(info->height*sizeof(PIXEL));

    imageS2 = (unsigned char **) calloc(info->width,sizeof(unsigned char*));
    for(int i=0; i<info->width; i++)
            imageS2[i] = (unsigned char*) calloc(info->height,sizeof(unsigned char));

    if(info->bits ==24){
        t=info->height-1;
        for(int i=0; i<info->height; i++){
            t=info->width-1;
            for(int j=0; j<info->width; j++){
                image2[t][i]=image[i][j];
                t--;
            }
        }

    }else if(info->bits == 8){

        t=info->height-1;   //horario
        for(int i=0; i<info->height; i++){
            t=info->width-1;
            for(int j=0; j<info->width; j++){
                imageS2[t][i]=imageS[i][j];
                t--;
            }
        }
    }
    w = info->height;
    h = info->width;
    info->height = h;
    info->width = w;
    w = info->xresolution;
    h = info->yresolution;
    info->xresolution = h;
    info->yresolution = w;

    npadding = (info->width*3) % 4;
    image = image2;
    imageS = imageS2;
}

void espejoY(INFOHEADER *info){
    int t;
   PIXEL **image2;
   unsigned char **imageS2;

   if(info->bits == 24){
        image2 =(PIXEL**) malloc(info->height*sizeof(PIXEL*));
        for(int i=0; i<info->height; i++)
            image2[i] = (PIXEL*) malloc(info->width*sizeof(PIXEL));



        for(int i=0; i<info->height; i++){
            t=info->width-1;
            for(int j=0; j<info->width; j++){
                image2[i][t]=image[i][j];
                t--;
            }
        }
        image = image2;
    }else if(info->bits == 8){
        imageS2 =(byte**) malloc(info->height*sizeof(byte*));
        for(int i=0; i<info->height; i++)
            imageS2[i] = (byte*) malloc(info->width*sizeof(byte));

        for(int i=0; i<info->height; i++){
            t=info->width-1;
            for(int j=0; j<info->width; j++){
                imageS2[i][t]=imageS[i][j];
                t--;
            }
        }
        imageS = imageS2;
    }
}

void espejoX(INFOHEADER *info){
    unsigned char **imageS2;
    PIXEL **image2;
    int t;
    //free(imageS2);
    //free(image2);
    if(info->bits==24){
        image2 =(PIXEL**) malloc(info->height*sizeof(PIXEL*));
        for(int i=0; i<info->height; i++)
            image2[i] = (PIXEL*) malloc(info->width*sizeof(PIXEL));

        t=info->height-1;
        for(int i=0; i<info->height; i++){
            for(int j=0; j<info->width; j++)
                image2[t][j]=image[i][j];
            t--;
        }
        image = image2;
    }else if(info->bits==8){
        imageS2 =(byte**) malloc(info->height*sizeof(byte*));
        for(int i=0; i<info->height; i++)
            imageS2[i] = (byte*) malloc(info->width*sizeof(byte));

        t=info->height-1;
        for(int i=0; i<info->height; i++){
            for(int j=0; j<info->width; j++)
                imageS2[t][j]=imageS[i][j];
            t--;
        }
        imageS = imageS2;
    }
}

void displayInfo(HEADER *header, INFOHEADER *info){
    printf("\nInformacion de la imagen:\n");
    printf("HEADER size: %d bytes\n ", sizeof(unsigned short int)+sizeof(unsigned int)+sizeof(unsigned short int)+sizeof(unsigned short int)+sizeof(unsigned int));
    printf("Tipo de archivo: %c%c\n ", header->type[0], header->type[1]);
    printf("Tamano del archivo: %d bytes\n ", header->size);
    printf("Offset: %d bytes\n ", header->offset);
    printf("\n");

    printf("INFOHEADER size: %d bytes\n ", sizeof(INFOHEADER));
    printf("Tamano de la cabecera: %d bytes\n ", info->size);
    printf("Width/Ancho: %d px\n ", info->width);
    printf("Height/Alto: %d px\n ", info->height);
    printf("Planos: %d\n ", info->planes);
    printf("Bits per pixel %d\n ", info->bits);
    printf("Compresion: %d\n ", info->compression);
    printf("Tamaño de la imagen: %d bytes\n ", info->imagesize);
    printf("Resolucion en x: %d \n ", info->xresolution);
    printf("Resolucion en y: %d\n ", info->yresolution);
    printf("Numero de colores: %d\n ", info->ncolours);
    printf("Colores importantes %d\n ", info->importancolours);
    printf("\n");
}

int main(){
    HEADER header;
    INFOHEADER info;
    int op;
    boool = 0;

    printf("Bienvenido al lector de imagenes retro de para images con formato BMP!\n Por favor escriba el nombre la imagen que desea leer:\n Ejemplo: hola.bmp\n");
    scanf("%s", namef);

    LoadBMP(&header, &info);
    printf("\nImagen cargada!\n ");
    displayInfo(&header, &info);
    if(boool){
        info.bits=24;
        info.ncolours=0;
        info.importancolours=0;
        info.imagesize = info.width*info.height*3;
        header.size = info.imagesize + header.offset;
    }
    SaveBMP(&header, &info);
    printf("Por favor introduzca una opcion para editar la image:\n 1.- Negativo\n 2.-Rotacion 90 grados CW\n 3.-Rotacion 90 grados CCW\n 4.-Espejo en y\n 5.-Espejo en x\n 6.-Reiniciar imagen\n 7.-Cargar otra imagen\n 0.-Salir\n");
    while(1){
        scanf("%d", &op);
        if(op==7){
            free(image);
            free(imageS);
            boool = 0;
            printf("\nIntroduzca el nombre de la siguiente imagen a cargar\n");
            scanf("%s", namef);
            LoadBMP(&header, &info);
            printf("\nNueva imagen cargada!\n ");
            displayInfo(&header, &info);
            if(boool){
                info.bits=24;
                info.ncolours=0;
                info.importancolours=0;
                info.imagesize = info.width*info.height*3;
                header.size = info.imagesize + header.offset;
            }
            SaveBMP(&header, &info);
            printf("Por favor introduzca una opcion para editar la image:\n 1.- Negativo\n 2.-Rotacion CW\n 3.-Rotacion CCW\n 4.-Espejo en y\n 5.-Espejo en x\n 6.-Reiniciar imagen\n 7.-Cargar otra imagen\n 0.-Salir\n");
        }else if(op==6){
            boool = 0;

            LoadBMP(&header, &info);
        printf("\nImagen reiniciada!\n");
            if(boool){
                info.bits=24;
                info.ncolours=0;
                info.importancolours=0;
                info.imagesize = info.width*info.height*3;
                header.size = info.imagesize + header.offset;
            }
            SaveBMP(&header, &info);
        }else if(op==1){
            negative(&header, &info);
            SaveBMP(&header, &info);
        }else if(op == 2){
            rotatesC(&info);
            SaveBMP(&header, &info);
        }else if(op == 3){
            rotatesCC(&info);
            SaveBMP(&header, &info);
        }else if(op == 4){
            espejoY(&info);
            SaveBMP(&header, &info);
        }else if(op == 5){
            espejoX(&info);
            SaveBMP(&header, &info);
        }else if(!op)
            exit(0);
        else
            printf("Opcion no valida intente de nuevo :v\n");
    }
    return 0;
}
