#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <conio.h>
#include <string>
#include <ctime>
#define N 256


using namespace std;

void SP(char *X, char *key, char *res)
{
	int sigma[16] = { 4,10,9,2,13,8,0,14,6,11,1,12,7,15,5,3 };
	int byte_str[16] = { 0 };
	//char res[2];//16 бит
	unsigned char val[4];
	uint16_t sVal[4], P = 0, P_res = 0;//в P будут хранится биты для перстановки

	//XOR с ключом
	for (int i = 0; i < 2; i++)//16 бит
	{
		res[i] = (X[i]) ^ (key[i]);
	}
	//делим на m-фрагменты по 4 бита(полсимвола)

	//m-фрагменты
	val[0] = ((unsigned char)res[0]) >> 4;
	val[1] = (unsigned char)res[0] & 0xF;
	val[2] = ((unsigned char)res[1]) >> 4;
	val[3] = (unsigned char)res[1] & 0xF;


	//применяем подстановку S
	for (int i = 0; i < 4; i++)
	{
		sVal[i] = sigma[int(val[i])];
		P = P | (sVal[i] << (4 * i));//также заполняем P битами для перестановки(накладываем char'ы со смещением)
	}

	int new_pos;
	////
	//применяем перестановку P по 16 битам
	for (int i = 0; i < 16; i++)
	{
		if (((P >> (15 - i)) & 1u) != 0)//если i-ый бит числа P равен единице
		{
			new_pos = 15 - (13 * i + 7) % 16;
			P_res = P_res | (1 << new_pos);//устанавливаем едиинцу в нужный бит
		}

	}

	//*(unsigned int *)res = P_res;
	*(uint16_t *)res = (uint16_t)P_res;

}

void Feistel(char *X1,char *X2,char *key)
{
	char feist_res[2];
	char SP_res[2];
	SP(X2, key, SP_res);
	for (int j = 0; j < 2; j++)
	{
		X1[j] = ((X1[j]) ^ (SP_res[j]));
	}
}

void Massey(char *X1, char *X2, char *key)
{
	char feist_res[2];
	char SP_res[2], X3[2];

	for (int j = 0; j < 2; j++)
	{
		X3[j] = X1[j] ^ X2[j];
	}
	SP(X3, key, SP_res);
	for (int j = 0; j < 2; j++)
	{
		X1[j] = X1[j] ^ SP_res[j];
		X2[j] = X2[j] ^ SP_res[j];
	}

}

void CryptoSystemEncrypt(char *X1, char *X2, char *key1, char *key2)
{
	//1-ый такт
	Feistel(X1, X2, key1);
	//2-ой такт(тут меняются порядком X1 и X2, так как конкатенация [ X2 || (....) ] )
	Feistel(X2, X1, key2);
	//3-ий такт
	Massey(X1, X2, key1);
	//4-ый такт
	Massey(X1, X2, key2);
}

void CryptoSystemDecrypt(char *X1, char *X2, char *key1, char *key2)
{

	Massey(X1, X2, key2);
	Massey(X1, X2, key1);
	Feistel(X2, X1, key2);
	Feistel(X1, X2, key1);

}

int main()
{
		
	char nul_symbol = NULL;
	char X1[2], X2[2], X3[2];


	//считывание ОТ с файла
	char openText[N];
	int openText_length = 0;
	FILE *input = fopen("in.txt", "rb");
	while (!feof(input))
	{
		fscanf(input, "%c", &openText[openText_length]);
		openText_length++;
	}
	openText_length--;//последний символ
	fclose(input);
	///////////////////////

	char key[4];
	int key_length = 0;
	FILE *key_file = fopen("key.txt", "rb");
	while (!feof(key_file))
	{
		fscanf(key_file, "%c", &key[key_length]);
		key_length++;
	}
	key_length--;//последний символ
	fclose(key_file);

	//делим ключ на половинки
	char key1[2], key2[2];
	for (int i = 0; i < 2; i++)
	{
		key1[i] = key[i];
		key2[i] = key[i + 2];
	}

	int value_block;
	//ДОПОЛНЕНИЕ БЛОКА
	//если длина ОТ кратна 32 битам, то добавляем нулевой блок
	if (openText_length % 4 == 0)//кратен 32
	{
		for (int i = 0; i < 4; i++)
		{
			openText[openText_length + i] = nul_symbol;
		}
		openText_length += 4;
	}
	else if (openText_length % 4 != 0)//не кратен 32
	{
		//дополняем нулями до длины 32
		value_block = (openText_length % 4) * 8;
		for (int i = 0; i < 4 - openText_length % 4; i++)
		{
			openText[openText_length + i] = nul_symbol;
		}
		openText_length = openText_length + (4 - openText_length % 4);
		//
		for (int i = 0; i < 3; i++)
		{
			openText[openText_length + i] = nul_symbol;
		}
		openText[openText_length + 3] = value_block;
		openText_length += 4;

	}

	char *SP_res = (char*)malloc(2 * sizeof(char));
	char feist_res[2], concat_res[4];
	char massey_res1[2], massey_res2[2];


	int nnum;// = 7;// = 12;
	int round_number;
	printf("1)Encrypt\n");
	printf("2)Decrypt\n");

	printf("3)Weak keys\n");
	printf("4)Check weak keys\n");

	printf("5)Half-weak keys\n");
	printf("6)Check half-weak keys\n");
	printf("7)Time error prop\n");
	cin >> nnum;
	//Encrypt
	if (nnum == 1)
	{
		FILE *encrypt = fopen("encrypted.txt", "wb");
		for (int i = 0; i < openText_length / 4; i++)//по всем 32-битным блокам
		{
			//1-st round Feistel
			for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
			{
				X1[j] = openText[i * 4 + j];
				X2[j] = openText[i * 4 + 2 + j];
			}

			CryptoSystemEncrypt(X1, X2, key1, key2);

			//записываем в файл
			for (int j = 0; j < 2; j++)
				fprintf(encrypt, "%c", X1[j]);
			for (int j = 0; j < 2; j++)
				fprintf(encrypt, "%c", X2[j]);

		}
		fclose(encrypt);
	}
	//Decrypt
	if (nnum == 2)
	{
		int l;//длина l для дополнения блока
		//считывание ОТ с файла
		char cypherText[N], decypherText[N];
		int cypherText_length = 0, decypherText_length = 0;
		FILE *decrypting_input = fopen("encrypted.txt", "rb");
		while (!feof(decrypting_input))
		{
			fscanf(decrypting_input, "%c", &cypherText[cypherText_length]);
			cypherText_length++;
		}
		cypherText_length--;//последний символ
		fclose(decrypting_input);
		///////////////////////

		FILE *decrypt = fopen("decrypted.txt", "wb");
		for (int i = 0; i < openText_length / 4; i++)//по всем 32-битным блокам
		{
			//1-st round Feistel
			for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
			{
				X1[j] = cypherText[i * 4 + j];
				X2[j] = cypherText[i * 4 + 2 + j];
			}

			CryptoSystemDecrypt(X1, X2, key1, key2);

			//заполняем буфер decypherText
			for (int j = 0; j < 2; j++)
			{
				decypherText[decypherText_length] = X1[j];
				decypherText_length++;
			}
			for (int j = 0; j < 2; j++)
			{
				decypherText[decypherText_length] = X2[j];
				decypherText_length++;
			}

		}
		//decypherText_length--;//последний символ 
		//определяем по последнему байту длину l
		l = ((int)(decypherText[decypherText_length - 1])) / 8;
		//вычитаем из исходной длины (последний блок + n-1)
		if (l != 0)
		{
			decypherText_length = decypherText_length - (4 + (4 - l));
		}
		else
		{
			decypherText_length = decypherText_length - 4;
		}

		//записываем в файл
		for (int i = 0; i < decypherText_length; i++)
		{
			fprintf(decrypt, "%c", decypherText[i]);
		}




		fclose(decrypt);
	}
	//Weak keys
	if (nnum == 3)
	{
		unsigned int step_32 = pow(2, 32), step_31 = pow(2, 31), step_30 = pow(2, 30), step_28 = pow(2, 28);
		unsigned int step_26 = pow(2, 26), step_24 = pow(2, 24), step_18 = pow(2, 18), step_8 = pow(2, 8);
		for (unsigned long long i = 0; i <= step_32 - 1; i++)
		{
			if (i == step_18)
				printf("i = 2^18\n");
			if (i == step_24)
				printf("i = 2^24\n");
			if (i == step_26)
				printf("i = 2^26\n");
			if (i == step_28)
				printf("i = 2^28\n");
			if (i == step_30)
				printf("i = 2^30\n");
			if (i == step_31)
				printf("i = 2^31\n");
			if (i == step_32 - 1)
				printf("i = 2^32\n");

			*(unsigned int *)key = i;
			for (int j = 0; j < 2; j++)
			{
				key1[j] = key[j];
				key2[j] = key[j + 2];
			}

			for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
			{
				X1[j] = openText[j];
				X2[j] = openText[2 + j];
			}
			CryptoSystemEncrypt(X1, X2, key1, key2);
			CryptoSystemEncrypt(X1, X2, key1, key2);
			//сравниваем получившийся результат с исходным ОТ
			int flag_slab = 0;
			for (int j = 0; j < 2; j++)
			{
				if (openText[j] != X1[j])
				{
					flag_slab = 1;
					break;
				}
			}
			for (int j = 0; j < 2; j++)
			{
				if (openText[j + 2] != X2[j])
				{
					flag_slab = 1;
					break;
				}
			}

			if (flag_slab == 0)
			{
				printf("Weak Key!!\n");
				printf("key_value = %u\n", i);
			}
		}

	}
	//Check weak keys
	if (nnum == 4)
	{
		FILE *encrypt_twice_weak = fopen("encrypted_twice_weak.txt", "wb");
		//*(unsigned int *)key = 72992054;
		*(unsigned int *)key = 284855846;
		//*(unsigned int *)key = 128852928;
		for (int j = 0; j < 2; j++)
		{
			key1[j] = key[j];
			key2[j] = key[j + 2];
		}
		for (int i = 0; i < openText_length / 4; i++)//по всем блокам
		{
			
			for (int j = 0; j < 2; j++)
			{
				X1[j] = openText[i * 4 + j];
				X2[j] = openText[i * 4 + 2 + j];
			}
			//////ШИФРУЕМ ПЕРВЫЙ РАЗ 
			CryptoSystemEncrypt(X1, X2, key1, key2);

			//////ШИФРУЕМ ПОВТОРНО
			CryptoSystemEncrypt(X1, X2, key1, key2);


			//
			for (int i = 0; i < 2; i++)
				fprintf(encrypt_twice_weak, "%c", X1[i]);
			for (int i = 0; i < 2; i++)
				fprintf(encrypt_twice_weak, "%c", X2[i]);
		}
		fclose(encrypt_twice_weak);

	}
	//Half-weak keys
	if (nnum == 5)
	{
		char X1k[2], X2k[2];
		unsigned int step_32 = pow(2, 32), step_31 = pow(2, 31), step_30 = pow(2, 30), step_28 = pow(2, 28);
		unsigned int step_26 = pow(2, 26), step_24 = pow(2, 24), step_18 = pow(2, 18), step_8 = pow(2, 8);
		//for (unsigned long long i = 0; i <= step_32 - 1; i++)
		for (unsigned long long i = step_32 - 1; i > 0 ; i--)
		{
			if (i == step_18)
				printf("i = 2^18\n");
			if (i == step_24)
				printf("i = 2^24\n");
			if (i == step_26)
				printf("i = 2^26\n");
			if (i == step_28)
				printf("i = 2^28\n");
			if (i == step_30)
				printf("i = 2^30\n");
			if (i == step_31)
				printf("i = 2^31\n");
			if (i == step_32-1)
				printf("i = 2^32\n");

			*(unsigned int *)key = i;
			for (int j = 0; j < 2; j++)
			{
				key1[j] = key[j];
				key2[j] = key[j + 2];
			}

			//ШИФРУЕМ
			//1-st round Feistel
			for (int j = 0; j < 2; j++)//
			{
				X1[j] = openText[j];
				X2[j] = openText[2 + j];
			}
			CryptoSystemEncrypt(X1, X2, key1, key2);



			//for (unsigned long long k = 0; k <= step_32 - 1; k++)
			for (unsigned long long k = step_32 - 1; k > 0; k--)
			{
				if (k == step_18)
					printf("k = 2^18\n");
				if (k == step_24)
					printf("k = 2^24\n");
				if (k == step_26)
					printf("k = 2^26\n");
				if (k == step_28)
					printf("k = 2^28\n");
				if (k == step_30)
					printf("k = 2^30\n");
				if (k == step_31)
					printf("k = 2^31\n");
				if (k == step_32-1)
					printf("k = 2^32\n");

				*(unsigned int *)key = k;
				for (int j = 0; j < 2; j++)
				{
					key1[j] = key[j];
					key2[j] = key[j + 2];
				}

				//чтобы не значения X1,X2 не перезаписывались в цикле k
				for (int j = 0; j < 2; j++)
				{
					X1k[j] = X1[j];
					X2k[j] = X2[j];
				}

				//ШИФРУЕМ ПОВТОРНО
				CryptoSystemEncrypt(X1k, X2k, key1, key2);


				//сравниваем получивший результат с исходным ОТ
				int flag_slab = 0;
				for (int j = 0; j < 2; j++)
				{
					if (openText[j] != X1k[j])
					{
						flag_slab = 1;
						break;
					}
				}
				for (int j = 0; j < 2; j++)
				{
					if (openText[j+2] != X2k[j])
					{
						flag_slab = 1;
						break;
					}
				}

				if (flag_slab == 0)
				{
					printf("Half-weak Key!!\n");
					printf("key_value: i = %llu, k = %llu\n", i, k);
				}
			}
		}


	}
	//Check half-weak keys
	if (nnum == 6)
	{
		FILE *encrypt_twice_half_weak = fopen("encrypted_twice_half_weak.txt", "wb");
		//*(int *)key = 72992054;
		//*(int *)key = 284855846;
		//i = 4294967292, k = 1948898053
		for (int i = 0; i < openText_length / 4; i++)//по всем блокам
		{

			*(unsigned int *)key = 4294967292;
			for (int j = 0; j < 2; j++)
			{
				key1[j] = key[j];
				key2[j] = key[j + 2];
			}
			//////ШИФРУЕМ ПЕРВЫЙ РАЗ
			for (int j = 0; j < 2; j++)// 
			{
				X1[j] = openText[i * 4 + j];
				X2[j] = openText[i * 4 + 2 + j];
			}
			CryptoSystemEncrypt(X1, X2, key1, key2);
			
			//ШИФРУЕМ ВТОРОЙ РАЗ
			*(unsigned int *)key = 1948898053;
			for (int j = 0; j < 2; j++)
			{
				key1[j] = key[j];
				key2[j] = key[j + 2];
			}

			//////ШИФРУЕМ ВТОРОЙ РАЗ 
			//1-st round Feistel
			CryptoSystemEncrypt(X1, X2, key1, key2);


			// 
			for (int i = 0; i < 2; i++)
				fprintf(encrypt_twice_half_weak, "%c", X1[i]);
			for (int i = 0; i < 2; i++)
				fprintf(encrypt_twice_half_weak, "%c", X2[i]);
			
		}
		fclose(encrypt_twice_half_weak);

	}
	//Error prop
	if (nnum == 7)
	{
		printf("Print round number\n");
		cin >> round_number;
		//round_number = 1;
		int temp_res1, temp_res2, check_razr;
		char temp1[N], temp2[N], result_err1[N], result_err2[N];
		unsigned char res_xor[4];
		unsigned int perem_razmn_err = 0, deg_num;
		int error_prop_flag = 0;

		unsigned int two_32 = -1;
		unsigned int step_31 = pow(2, 31), step_30 = pow(2, 30), step_28 = pow(2, 28);
		unsigned int step_26 = pow(2, 26), step_24 = pow(2, 24), step_18 = pow(2, 18);

		for (int j = 0; j < 32; j++)//фиксируем определнный бит = степень двойки от 2^0 до 2^31
		{
			printf("bit number = %i\n", j);
			//printf("perem value = %u\n", perem_razmn_err);
			deg_num = pow(2, j);
			error_prop_flag = 0;
			perem_razmn_err = 0;
			for (unsigned long long i = 0; i <= two_32; i++)//будем прибавлять числовым значением
			//for (unsigned long long i = two_32; i >0 ; i--)//будем прибавлять числовым значением
			{
				if (i == step_18)
				{
					printf("i = 2^18\n");
					printf("perem value = %u\n", perem_razmn_err);
				}
				if (i == step_24)
				{
					printf("i = 2^24\n");
					printf("perem value = %u\n", perem_razmn_err);
				}
				if (i == step_26)
				{
					printf("i = 2^26\n");
					printf("perem value = %u\n", perem_razmn_err);
				}
				if (i == step_28)
				{
					printf("i = 2^28\n");
					printf("perem value = %u\n", perem_razmn_err);
				}
				if (i == step_30)
				{
					printf("i = 2^30\n");
					printf("perem value = %u\n", perem_razmn_err);
				}
				if (i == step_31)
				{
					printf("i = 2^31\n");
					printf("perem value = %u\n", perem_razmn_err);
				}


				if (((i >> j) & 1u) != 0)//j-ый бит числа зафиксирован (установлен в 1)
				{

					*(unsigned int *)temp1 = i;
					*(unsigned int *)temp2 = i - deg_num;
					//шифруем
					//Первый такт
					if (round_number >= 1)
					{
						//1-st round Feistel, temp1
						{
							for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
							{
								X1[j] = temp1[j];
								X2[j] = temp1[2 + j];
							}
							SP(X2, key1, SP_res);
							//X1 xor fk(X2)
							for (int j = 0; j < 2; j++)
							{
								feist_res[j] = ((X1[j]) ^ (SP_res[j]));
							}
							//конатенация с X2
							for (int j = 0; j < 2; j++)
							{
								result_err1[j] = X2[j];
								result_err1[j + 2] = feist_res[j];
							}
						}

						//1-st round Feistel, temp2
						{
							for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
							{
								X1[j] = temp2[j];
								X2[j] = temp2[2 + j];
							}
							SP(X2, key1, SP_res);
							//X1 xor fk(X2)
							for (int j = 0; j < 2; j++)
							{
								feist_res[j] = ((X1[j]) ^ (SP_res[j]));
							}
							//конатенация с X2
							for (int j = 0; j < 2; j++)
							{
								result_err2[j] = X2[j];
								result_err2[j + 2] = feist_res[j];
							}
						}
					}
					//Второй такт
					if (round_number >= 2)
					{
						//2-st round Feistel, temp 1
						{
							for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
							{
								X1[j] = result_err1[j];
								X2[j] = result_err1[2 + j];
							}
							SP(X2, key2, SP_res);
							//X1 xor fk(X2)
							for (int j = 0; j < 2; j++)
							{
								feist_res[j] = (X1[j]) ^ (SP_res[j]);
							}
							//конатенация с X2
							for (int j = 0; j < 2; j++)
							{
								result_err1[j] = X2[j];
								result_err1[j + 2] = feist_res[j];
							}
						}

						//2-st round Feistel, temp 2
						{
							for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
							{
								X1[j] = result_err2[j];
								X2[j] = result_err2[2 + j];
							}
							SP(X2, key2, SP_res);
							//X1 xor fk(X2)
							for (int j = 0; j < 2; j++)
							{
								feist_res[j] = (X1[j]) ^ (SP_res[j]);
							}
							//конатенация с X2
							for (int j = 0; j < 2; j++)
							{
								result_err2[j] = X2[j];
								result_err2[j + 2] = feist_res[j];
							}
						}
					}
					//Третий такт
					if (round_number >= 3)
					{
						//3-rd round Massey, temp1 
						{
							for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
							{
								X1[j] = result_err1[j];
								X2[j] = result_err1[2 + j];
							}

							for (int j = 0; j < 2; j++)
							{
								X3[j] = X1[j] ^ X2[j];
							}
							//fk(X1 xor X2) = fk(X3)
							SP(X3, key1, SP_res);
							for (int j = 0; j < 2; j++)
							{
								massey_res1[j] = X1[j] ^ SP_res[j];
								massey_res2[j] = X2[j] ^ SP_res[j];
							}
							//конкатенация 
							for (int j = 0; j < 2; j++)
							{
								result_err1[j] = massey_res1[j];
								result_err1[j + 2] = massey_res2[j];
							}
						}

						//3-rd round Massey, temp2 
						{
							for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
							{
								X1[j] = result_err2[j];
								X2[j] = result_err2[2 + j];
							}

							for (int j = 0; j < 2; j++)
							{
								X3[j] = X1[j] ^ X2[j];
							}
							//fk(X1 xor X2) = fk(X3)
							SP(X3, key1, SP_res);
							for (int j = 0; j < 2; j++)
							{
								massey_res1[j] = X1[j] ^ SP_res[j];
								massey_res2[j] = X2[j] ^ SP_res[j];
							}
							//конкатенация 
							for (int j = 0; j < 2; j++)
							{
								result_err2[j] = massey_res1[j];
								result_err2[j + 2] = massey_res2[j];
							}
						}
					}
					//Четветрый такт
					if (round_number == 4)
					{
						//4-rd round Massey, temp 1
						{
							for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
							{
								X1[j] = result_err1[j];
								X2[j] = result_err1[2 + j];
							}
							for (int j = 0; j < 2; j++)
							{
								X3[j] = X1[j] ^ X2[j];
							}
							//fk(X1 xor X2) = fk(X3)
							SP(X3, key2, SP_res);
							for (int j = 0; j < 2; j++)
							{
								massey_res1[j] = X1[j] ^ SP_res[j];
								massey_res2[j] = X2[j] ^ SP_res[j];
							}
							//конкатенация 
							for (int j = 0; j < 2; j++)
							{
								result_err1[j] = massey_res1[j];
								result_err1[j + 2] = massey_res2[j];
							}
						}

						//4-rd round Massey, temp 2
						{
							for (int j = 0; j < 2; j++)//половинки => 16-бит(2 символа по 8 бит)
							{
								X1[j] = result_err2[j];
								X2[j] = result_err2[2 + j];
							}
							for (int j = 0; j < 2; j++)
							{
								X3[j] = X1[j] ^ X2[j];
							}
							//fk(X1 xor X2) = fk(X3)
							SP(X3, key2, SP_res);
							for (int j = 0; j < 2; j++)
							{
								massey_res1[j] = X1[j] ^ SP_res[j];
								massey_res2[j] = X2[j] ^ SP_res[j];
							}
							//конкатенация 
							for (int j = 0; j < 2; j++)
							{
								result_err2[j] = massey_res1[j];
								result_err2[j + 2] = massey_res2[j];
							}
						}
					}

					//ксорим результаты
					for (int j = 0; j < 4; j++)
					{
						res_xor[j] = result_err1[j] ^ result_err2[j];
					}

					unsigned int int_res_xor = *((unsigned int *)res_xor);
					perem_razmn_err = perem_razmn_err | int_res_xor;
					if (perem_razmn_err == two_32)
					{
						printf("ERROR PROPAGATION!\n");
						printf("perem value = %llu\n", perem_razmn_err);
						error_prop_flag = 1;

						break;
					}


				}

			}
			if (error_prop_flag == 0)
			{
				printf("NO ERROR PROPAGATION!\n");
				printf("perem value = %u\n", perem_razmn_err);
				break;
			}
		}

	}
	
	system("pause");
}