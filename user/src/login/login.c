#include "types.h"
#include "user.h"
#include "login.h"

/**
 * Prompt and get input from console
 * 
 * @param prompt Console output to display to user to prompt a response
 * @return heap allocated input response if success, else (char *) 0x0
 */
char *get_input(const char *prompt) {
  char buf[MAX_INPUT_SIZE];
  memset(buf, 0, sizeof(MAX_INPUT_SIZE));

  printf(1, "%s", prompt);
  gets(buf, MAX_INPUT_SIZE);

  if (buf[0] != 0) {
    buf[MAX_INPUT_SIZE - 1] = 0;
    uint input_length = strlen(buf);
    char *input = malloc(input_length);
    if (input != (char *) 0x0) {
      memmove(input, buf, input_length - 1);
      input[input_length - 1] = 0;
      return input;
    }
  }

  return (char *) 0x0;
}

static inline void assert_char_ptr(char *data, const char *err) {
  if (data == (char *) 0x0) {
    printf(2, "%s\n", err);
    exit();
  }
}

int main(void) {
  printf(1, "creating two new users");
  create_user("user1", "123123");
  create_user("user2", "456456");
  

  char *username = get_input("\nxv6 login: ");
  assert_char_ptr(username, "Unable to get username");

  // check if user exists or prompt to create new user
  char *password;
  if (does_user_exist(username) < 0) {
    password = get_input("User not found. Create with a password: ");
    assert_char_ptr(password, "Unable to get password");
    if (create_user(username, password) < 0) {
      printf(2, "login: unable to create user '%s' with password '%s'\n",
             username, password);
      exit();
    }
  } else {
    password = get_input("password: ");
    assert_char_ptr(password, "Unable to get password");
  }

  login_user(username, password);
  printf(2, "login: login_user failed to launch shell for user '%s' with "
         "password '%s'\n", username, password);
  exit();

  return 0;
}
