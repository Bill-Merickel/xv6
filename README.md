# User-Isolation and File Permissions

Here, I created a simple, but realistic user system, a file permission system, and a
a user-space login system.

## Design of xv6 Login System

### Security Mechanism Design

The security of our design is constructed as follows. A struct called User was created in the login_lib.c file, and the User object contains an integer called uid, an unsigned char called username, and an unsigned char called password. This struct will be used throughout the four methods in this file to easily access these properties within the file containing this user data. The user id, username, and encrypted password for every user is stored in a file at the path "/passwd" to ensure that this data remains persistent across reboots. To create a user, the create_user method first verifies that the username and password parameters do not exceed the maximum input size, as well as verifies that the user does not already exist (since we cannot change the password of a user once it is set). If we have passed these conditional checks, we begin by creating a new User struct called user, which will store the username, encrypted password, and uid of the user. The username is not hidden, hashed, or encoded in any way, so we simply pass in the username parameter to the user object. The password requires more computational logic to ensure its security however. To protect against rainbow tables, we will utilize salts to further add an element of randomness to the encrypted password list. For this program, we decided to mash the keyboard and generate a lengthy salt of random characters in the English alphabet, and we hashed this salt to further tighten its security. Note that every password will have the same salt (as this is stored locally within the create_user method).

Now that we have a hashed salt, we then hash our password and perform a series of string logic combinations to insert the hashed password in between the 10th and 11th characters of the hashed salt. This is particularly beneficial as we are essentially splitting our hashed salt and inserting one part in front of our hashed password and one part behind our hashed password, so it essentially serves as two salts instead of one (double the security!!). One our final concatenated password entry has been generated, we store it into the password field of our user struct. And finally, for our user id, we ensure that we are not assigning a duplicate user id to our newly created user by iterating through the list of already existing users and taking the last user's uid (which will be the highest). We retrieve this value, increment it by one, and then assign it to the uid parameter of the user struct. This ensures that every new user has an id that is 1 greater than the user id of the previous user to be created (and thus, all our user ids are unique). Once our user struct properties have been completely populated, we write this user struct into the "/passwd" file by appending it to the existing entries in this file.

When it comes to logging in users, we hash our password and perform the same string combination logic with the same salt to create the final concatenated password as we do during our create_user method, so we have the same security protections as we do when creating a user. We can compare this concatenated password to the one stored in "/passwd" and allow the user to log in if the two match. Retrieving a correct password from our combined hashed salt and password is going to be insanely difficult due to the split of the salts before and after the hashed password, so good luck trying to decipher what the actual password is by looking at the one saved in the "/passwd" file. To ensure that users don't get access to root permissions, we have created a separate account, root, that has password "admin" (as specified in the readme). In reality, we would pick a more secure password, but for the sake of this exercise we left it at that. The root account is created once during the init_hook method, which ensures that that user exists and has a UID of 0 (which grants it special privileges). Consequent users that are created are ensured to have a UID that is greater than the last user created, so the first "real" user created will have a UID of 1 and therefore not have that special UID that grants it additional permissions. By assigning the UIDs to new users this way, we are protecting against the special root privileges and ensure that only the root user is assigned to the UID of 0.

On a final note, the "/passwd" file is guaranteed to be protected from malicious user intentions (such as deleting the contents of the file) through the permissions assigned to the file. The file is created in init_hook(), which is called at the start of the xv6 boot process (the UID at this point is 0). When the file is created, the user with UID 0 is assigned as the owner, so they are the only individuals that can write this file. Note that in init_hook(), we are also creating our root user, which is the only user with UID 0 because of our incremental UID assignments as users are created. Therefore, the root user is the owner of the file and no other users have permissions to overwrite the data within this file, so the init_hook() implementation protects against this malicious behavior. Through this series of password hashing, salt hashing and splitting, and incremental UID assignments, we are ensuring that our XV6 login system contains a tight level of security and minimizes the potential vulnerabilities commonly associated with frequent password compromises.

### Files Created

The only file we create is "/passwd", which holds a list of each user's UID, username, and encrypted password. Note that when we say encrypted password, we did not actually use any real encryption method given to us from the lab. We just used hashing and string concatenation (combination) with salts. Reading this file will allow you to see which users have which UIDs, but the passwords will be nearly indecipherable.

## Adding Users

I first provided each process with a
logical user identifer (henceforth `uid`). There will be two functions which
allow a user-space process within xv6 to modify or view the current uid `uid`:

```c
/**
 * Sets the uid of the current process
 * 
 * @param uid The uid to change to. Valid ranges are 0x0 - 0xFFFF
 * @returns 0 on success, -1 on failure
 */
int setuid(int);

/**
 * Gets the uid of the currently running process.
 *
 * @returns The current process's uid.
 */
int getuid(void);
```

Beyond this description the following rules should be applied for uids:

- The uid of `init` is 0 and is considered the root process.
- Each child process inherits its parent's `uid`
- Only processes with `uid` 0 may change their `uid` through `setuid`

## File Permissions

I also added file
permissions to the system which leverage the `uid` system to logically isolate
certain files.

To accomplish this, you will add several properties to each file and directory
within xv6. First, the `owner` field will determine which uid `owns` a specific
file. Second, the `permission` field will determine the access privileges
non-owners have for a specific file.

### Logical Permissions

The permission system logically works in the following way:

- If a file is accessed by its owner or `uid` 0 (henceforth `root`), then that
  access is permitted
- If the file is accessed for reading by another user (not its owner or root)
  and its read permission is set, that access is permitted
- If the file is accessed for writing by another user (not its owner or root)
  and its write permission is set, that access is permitted
- All other accesses are not permitted.

You will need to identify any high-level operations which involve file accesses.
If an operation involves this, you must confirm the specific access type
performed is permitted for the given process. A sample of operations involving
file accesses are included below:

- directory reads (read)
- open (read and/or write, depending on the flags)
- file creation (write to file's directory)
- exec (read)

If an operation is not permitted, the system call should return -1, and no
changes to the disk or file-system state should occur. If the operation is
permitted, the operation should occur as they did before the permission system
was added.

## Login

In xv6, the initial user-space process launches the shell directly without
considering what user is currently logged in. In this part, I added login functionality by leveraging the first two components in addition to provided
user-space cryptographic tools for hashing (`sha256`) and encrypting
(`aes256_{encrypt,decrypt}`) available in `user/include/crypto.h`.

The lifecycle of the login system
look like the following:

```txt
init:
  [init_hook()]
  <infinite loop> {
    login:
      <fetch username from console>
      if [does_user_exist(username)] {
        <prompt for password>
      } else {
        <prompt for new password>
        [create_user(username, password)]
      }
      [login_user(username, password)]
  }
```

```c
/**
 * Hook into user/src/login/login_init.c in order to intialize any files or
 * data structures necessary for the login system
 * 
 * Called once per boot of xv6
 */
void init_hook();

/**
 * Check if user exists in system 
 * 
 * @param username A null-terminated string representing the username
 * @return 0 on success if user exists, -1 for failure otherwise
 */
int does_user_exist(char *username);

/**
 * Create a user in the system associated with the username and password. Cannot
 * overwrite an existing username with a new password. Expectation is for
 * created users to have a unique non-root uid.
 * 
 * @param username A null-terminated string representing the username
 * @param password A null-terminated string representing the password
 * @return 0 on success, -1 for failure
 */
int create_user(char *username, char *password);

/**
 * Login a user in the system associated with the username and password. Launch
 * the shell under the right permissions for the user. If no such user exists
 * or the password is incorrect, then login will fail.
 * 
 * @param username A null-terminated string representing the username
 * @param password A null-terminated string representing the password
 * @return no return on success, -1 for failure
 */
int login_user(char *username, char *password);
```

