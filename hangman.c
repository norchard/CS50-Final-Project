#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"

static void clear_infile(FILE* infile);
static bool get_word(FILE* infile, FILE* outfile);
static void draw_board(FILE* outfile);
static bool check_if_won(void);
static int get_letter(FILE* infile, FILE* outfile);
static void process_letter(char letter);

/* When a valid letter is guessed, it is added to the `guessed` array
   If the letter is in `word`, the most significant bit is set
   for the guessed character byte in both `word` and the `guessed` array.
   The set top bit is used to print only word characters that have been guessed
   and only "already guessed" characters that are not in `word`
*/
char* word;
char guessed[26];
int word_length, guesses, guessed_index;

// Main routine
int play_hangman(int infd, int outfd){
  memset(guessed, 0, 20);
  guesses = 6;
  guessed_index = 0;

  FILE* infile = fdopen(infd, "r");
  FILE* outfile = fdopen(outfd, "w");
  int c;

  if(get_word(infile, outfile)){
    goto exit;
  }

  while(guesses){
    draw_board(outfile);

    if(check_if_won()){
      fprintf(outfile, GRN"Congrats, you won!\n"WHT);
      fflush(outfile);
      goto exit;
    }
    else{
      if((c = get_letter(infile, outfile)) == EOF){
        goto exit;
      }
      process_letter(c);
    }
  }

  draw_board(outfile);
  fprintf(outfile, RED"You lost.\n"WHT);
  fflush(outfile);
exit:
  free(word);
  word = NULL; // Set null pointer so that malloc'd memory can't be freed twice
               // Should probably remove variables from global namespace :/
  fclose(infile);
  fclose(outfile);
  return 0;
}

// Get word
static bool get_word(FILE* infile, FILE* outfile){
  char buffer[100];
  bool valid_word = false;
  fprintf(outfile, "Please enter a word or phrase: ");
  fflush(outfile);

  while(!valid_word){
    if((fscanf(infile, "%[^\n]", buffer)) == EOF){
        return 1;
    }
    fflush(infile);
    valid_word = true;

    // Check that all chars are alphabetic
    for(int i = 0; buffer[i] != 0; i++){
      if(!isalpha(buffer[i]) && buffer[i] != ' '){
        valid_word = false;
      }
    }

    // Proper usage
    if(!valid_word){
      clear_infile(infile);
      fprintf(outfile, "Invalid characters. Try again: ");
      fflush(outfile);
    }
  }

  word_length = strlen(buffer);

  word = malloc(word_length + 1);
  strcpy(word, buffer);

  for(int i = 0; i < word_length; i++){
    word[i] = toupper(word[i]);
  }

  clear_infile(infile);

  return 0;
}

// Draw the board
static void draw_board(FILE* outfile){

  // Draw guillotine
  fprintf(outfile, "   ______\n   |    ");
  if(guesses <= 5){
    fprintf(outfile, YEL"O"WHT);
  }
  fprintf(outfile, "\n   |   ");
  if(guesses == 4){
    fprintf(outfile, YEL" |"WHT);
  }
  else if(guesses == 3){
    fprintf(outfile, YEL"/|"WHT);
  }
  else if(guesses <= 2){
    fprintf(outfile, YEL"/|\\"WHT);
  }
  fprintf(outfile, "\n   |   ");
  if(guesses == 1){
    fprintf(outfile, YEL"/"WHT);
  }
  else if(guesses == 0){
    fprintf(outfile, YEL"/ \\"WHT);
  }
  fprintf(outfile, "\n __|______\n\n");

  // Draw word
  fprintf(outfile, CYN"Hint: ");
  for(int i = 0; i < word_length; i++){
    if(word[i] == ' '){
      fprintf(outfile, "  ");
    }
    else if((word[i] & 128) == 0){
      fprintf(outfile, "_ ");
    }
    else{
      fprintf(outfile, "%c ", (word[i] ^ 128));
    }
  }
  fprintf(outfile, "\n");

  // Draw guessed
  fprintf(outfile, BLU"Already guessed: ");

  for(int i = 0; guessed[i] != 0; i++){
    if((guessed[i] & 128) == 0 && isalpha(guessed[i])){
      fprintf(outfile, "%c ", guessed[i]);
    }
  }

  fprintf(outfile, "\n"WHT);
  fflush(outfile);

  return;
}

// Check if the game has been won
static bool check_if_won(void){
  for(int i = 0; i < word_length; i++){
    if((word[i] & 128) == 0 && isalpha(word[i])){
      return false;
    }
  }

  return true;
}

// Request a letter
static int get_letter(FILE* infile, FILE* outfile){
  int c = 0;
  fprintf(outfile, "Make a guess: ");
  fflush(outfile);

  while(1){
    c = getc(infile);

    if(c == EOF){
      return EOF;
    }

    if(getc(infile) != '\n'){
      fprintf(outfile, RED"\rEnter just one character. Try again: "WHT);
      fflush(outfile);
      clear_infile(infile);
    }
    else if(!isalpha(c)){
      fprintf(outfile, RED"\rInvalid character. Try again: "WHT);
      fflush(outfile);
    }
    else{
      c = toupper(c);
      for(int i = 0; guessed[i] != 0; i++){
        if((guessed[i] & 127) == c){
          c = 0;
        }
      }

      if(c == 0){
        fprintf(outfile, RED"\rAlready guessed. Try again: "WHT);
        fflush(outfile);
      }
      else{
        return c;
      }
    }
  }
}

// Check if letter is in word
static void process_letter(char letter){
  bool in_word = false;

  for(int i = 0; i < word_length; i++){
    if(word[i] == letter){
      word[i] ^= 128;
      in_word = true;
    }
  }

  if(!in_word){
    guesses--;
  }

  if(in_word){
    guessed[guessed_index] = letter ^ 128;
  }
  else{
    guessed[guessed_index] = letter;
  }

  guessed_index++;
}

// Clear stdin
static void clear_infile(FILE* infile){
  int c;
  while ((c = getc(infile)) != '\n'){
    if(c == EOF){
      return;
    }
  }
}
