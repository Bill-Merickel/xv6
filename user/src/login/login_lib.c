#include "types.h"
#include "user.h"
#include "login.h"
#include "crypto.h"
#include "fcntl.h"

struct User {
  int uid;
  uchar username[MAX_INPUT_SIZE + 1];
  uchar password[MAX_INPUT_SIZE + 1];
};

/**
 * Hook into user/src/login/login_init.c in order to intialize any files or
 * data structures necessary for the login system
 * 
 * Called once per boot of xv6
 */
void init_hook() {
  int passwordFD = open("/passwd", O_CREATE | O_RDWR);

  if(passwordFD == -1){
    close(passwordFD);
    exit();
  }
  
  close(passwordFD);
  create_user("root", "admin");
}


/**
 * Check if user exists in system 
 * 
 * @param username A null-terminated string representing the username
 * @return 0 on success if user exists, -1 for failure otherwise
 */
int does_user_exist(char *username) {
  // Read in the existing users in the passwd file
  int passwordFD = open("/passwd", O_RDWR);
  struct User users;
  int numOfCharactersRead = read(passwordFD, &users, sizeof(struct User));

  if (numOfCharactersRead == -1){
    close(passwordFD);
    return -1;
  }

  if (strcmp((const char*)username, (const char*)users.username) == 0){
    close(passwordFD);
    return 0;
  }

  while (numOfCharactersRead != 0) {
    numOfCharactersRead = read(passwordFD, &users, sizeof(struct User));
    if (strcmp((const char*)username, (const char*)users.username) == 0){
      close(passwordFD);
      return 0;
    }
  }

  close(passwordFD);
  return -1;
}

/**
 * Create a user in the system associated with the username and password. Cannot
 * overwrite an existing username with a new password. Expectation is for
 * created users to have a unique non-root uid.
 * 
 * @param username A null-terminated string representing the username
 * @param password A null-terminated string representing the password
 * @return 0 on success, -1 for failure
 */
int create_user(char *username, char *password) {
  if(strlen(username) > MAX_INPUT_SIZE || strlen(password) > MAX_INPUT_SIZE)
    return -1;

  if(does_user_exist(username) == 0)
    return -1;

  struct User user;
  strcpy((char*) user.username, (char*) username);

  // Very secure salt
  char *salt = "hifghidsyfgdfvgalsdicbvdalsycvglsuygvceiyfgvluy";
  char *hashed_salt[SHA256_SIZE_BYTES];
  sha256(salt, strlen(salt), (uchar*) hashed_salt);

  char *hashed_password[SHA256_SIZE_BYTES];
  sha256(password, strlen(password), (uchar*) hashed_password);

  char *password_entry[strlen((const char*)hashed_salt) + strlen((const char*)hashed_password)];

  strcpy((char*)password_entry, (const char*)hashed_salt);
  password_entry[10] = '\0';

  strcpy((char*)password_entry + strlen((const char*)password_entry), (const char*)hashed_password);
  strcpy((char*)password_entry + strlen((const char*)password_entry), (const char*)(hashed_salt + 10));
  strcpy((char*)user.password, (const char*)password_entry);
  
  // Read in the existing users in the passwd file
  int passwordFD = open("/passwd", O_RDWR);
  struct User users;
  int numOfCharactersRead = read(passwordFD, &users, sizeof(struct User));
  int maxUID = users.uid;

  if (numOfCharactersRead == -1){
    close(passwordFD);
    return -1;
  }
  
  while (numOfCharactersRead != 0) {
    numOfCharactersRead = read(passwordFD, &users, sizeof(struct User));
    maxUID = users.uid;
  }

  maxUID++;
  user.uid = maxUID;

  write(passwordFD, &user, sizeof(struct User));
  close(passwordFD);

  return 0;
}

/**
 * Login a user in the system associated with the username and password. Launch
 * the shell under the right permissions for the user. If no such user exists
 * or the password is incorrect, then login will fail.
 * 
 * @param username A null-terminated string representing the username
 * @param password A null-terminated string representing the password
 * @return no return on success, -1 for failure
 */
int login_user(char *username, char *password) {
  if(does_user_exist(username) == -1)
    return -1;

  // Very secure salt
  char *salt = "hifghidsyfgdfvgalsdicbvdalsycvglsuygvceiyfgvluy";
  char *hashed_salt[SHA256_SIZE_BYTES];
  sha256(salt, strlen(salt), (uchar*) hashed_salt);

  char *hashed_password[SHA256_SIZE_BYTES];
  sha256(password, strlen(password), (uchar*) hashed_password);

  char *password_entry[strlen((const char*)hashed_salt) + strlen((const char*)hashed_password)];

  strcpy((char*)password_entry, (const char*)hashed_salt);
  password_entry[10] = '\0';

  strcpy((char*)password_entry + strlen((const char*)password_entry), (const char*)hashed_password);
  strcpy((char*)password_entry + strlen((const char*)password_entry), (const char*)(hashed_salt + 10));

  char *encrypted_password = 0;
  strcpy((char*)encrypted_password, (const char*)password_entry);

  int passwordFD = open("/passwd", O_RDWR);
  struct User users;
  int numOfCharactersRead = read(passwordFD, &users, sizeof(struct User));

  if (numOfCharactersRead == -1){
    close(passwordFD);
    return -1;
  }

  if ((strcmp((const char*)username, (const char*)users.username) == 0) && (strcmp((const char*)encrypted_password, (const char*)users.password) == 0)){
    close(passwordFD);

    char *argv[] = { "sh", 0 };
    int pid, wpid;
    
    for(;;){
      printf(1, "init: starting sh\n");
      pid = fork();
      if(pid < 0){
        printf(1, "init: fork failed\n");
        exit();
      }
      if(pid == 0){
        setuid(users.uid);
        exec("sh", argv);
        printf(1, "init: exec sh failed\n");
        exit();
      }
      while((wpid=wait()) >= 0 && wpid != pid)
        printf(1, "zombie!\n");
    }
  }

  while (numOfCharactersRead != 0) {
    numOfCharactersRead = read(passwordFD, &users, sizeof(struct User));
    if ((strcmp((const char*)username, (const char*)users.username) == 0) && (strcmp((const char*)encrypted_password, (const char*)users.password) == 0)){
      close(passwordFD);

      char *argv[] = { "sh", 0 };
      int pid, wpid;
      
      for(;;){
        printf(1, "init: starting sh\n");
        pid = fork();
        if(pid < 0){
          printf(1, "init: fork failed\n");
          exit();
        }
        if(pid == 0){
          setuid(users.uid);
          exec("sh", argv);
          printf(1, "init: exec sh failed\n");
          exit();
        }
        while((wpid=wait()) >= 0 && wpid != pid)
          printf(1, "zombie!\n");
      }
    }
  }

  close(passwordFD);
  return -1;
}
