#include<string.h>
#include<time.h>

struct user{
	char username[50];
	char password[50];
	char enroll_date[11]; // yyyy-mm-dd
	char last_active_date[11]; 
};

char *users_file = "users.txt";

char *get_current_time(char *s, int size){
    time_t rawtime;
    struct tm * mytime;

    time ( &rawtime );
    mytime = localtime ( &rawtime );
    
    bzero(s, size);
    sprintf(s, "%d:%d:%d\n", mytime->tm_hour+2, mytime->tm_min, mytime->tm_sec);
    return s;
}

char *get_current_date(char *s, int size){
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    bzero(s, size);
    sprintf(s, "%d-%d-%d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
    return s;
}

//Should use free(u) after every call to this function!
struct user* search_user(char *username){
	FILE *f = fopen(users_file, "r");
	if(!f){
		printf("Error opening file for reading!");
		return NULL;
	}

	struct user* u = (struct user*)malloc(sizeof(struct user));
	char buf[200];

	while(!feof(f)){
		fgets(buf, sizeof(buf), f);

		char *token = strtok(buf, " ");

		if(strcmp(token, username) == 0){
			strcpy(u->username, token);
   			
      		token = strtok(NULL, " ");
      		strcpy(u->password, token);

      		token = strtok(NULL, " ");
      		strcpy(u->enroll_date, token);

      		token = strtok(NULL, " ");
      		strcpy(u->last_active_date, token);
		
			fclose(f);
			return u;
		}
	}

	fclose(f);
	return NULL;
}

int add_user(char *username, char *password, char *enroll_date, char *last_active_date){
	FILE *f = fopen(users_file, "a");
	if(!f){
		printf("Error opening file for append!");
		return -1;
	}

	struct user* u = search_user(username);
	if(u == NULL){
		//printf("ADDED %s\n", username);
		fprintf(f, "%s %s %s %s\n", username, password, enroll_date, last_active_date);
	}else{
		//printf("Username %s already used!\n", username);
		free(u);
		fclose(f);
		return -1;
	}

	free(u);
	fclose(f);
	return 1;
}

int check_password(char *username, char *password){
	struct user *u = search_user(username);
	char pw[50];
	strcpy(pw, u->password);
	free(u);

	return strcmp(pw, password) == 0 ? 1 : 0;
}
