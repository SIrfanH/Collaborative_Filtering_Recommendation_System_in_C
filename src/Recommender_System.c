#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define Size 1000
#define wSize 100




typedef struct Book     // Structure for storing user books
{
    char name[wSize];   // Name of the book
    short int rating;   // Rating of the book
}Book;

typedef struct Users    // Structure for storing users
{
    char name[wSize];   // Name of the user
    Book *books[wSize]; // Array of books
}Users;

typedef struct Similars // Structure for storing users similar to New User
{
    Users *user;        // Pointer to user that is similar to new user
    double simPoint;    // Similarity value of the user to new user
    double mean;        // Average ratings of the user
}Similars;

typedef struct NotRead  // Structure for storing Books not read by New User
{
    Book *notReadBook;      // Pointer to the book that is not read by the new user
    int notReadBookIndex;   // Index of the not read book for easy access
    double prediction;      // prediction value found by pred
}NotRead;


void nullify(Users *user);  // Function for initializing table of users as NULL
Users* create_user_node(char userName[]); // For creating new user node
Book* create_book_node(char name[], char rating); // For creating new Book node
int read_first_line(int index, char line[], char bookName[]); // For separating first line of CSV file which contains book names
void readCSV(Users *table[]); // For reading CSV file line by line and separating users and ratings from lines and creating a table of users
double pearson_coefficient(Users *userA, Users* userB); // For calculating similarity rate of two users
int search_for_index(Users* table[], char name[]); // For getting index of the New User from users table
void isGreater(Users* table[], int index, double sim, Similars* simls[], int k); // replace old value of similarity rate
void k_similar(Users *table[], int k, char name[]); // Function for finding K number of similar users to New User
void pred(Users *table[], int index, Similars *simls[]); // Function for predicting rating


void nullify(Users *user)
{
    int k;
    for(k=0;k<wSize;k++)
    {
        user->books[k] = NULL;
    }
}

// For creating new user node
Users* create_user_node(char userName[])
{
    Users *user = NULL;
    user = (Users*)malloc(sizeof(Users));
    strcpy(user->name,userName);
    nullify(user);
    return user;
}

// For creating new Book node
Book* create_book_node(char name[], char rating)
{
    Book *pointer = NULL;
    pointer = (Book*)malloc(sizeof(Book));
    strcpy(pointer->name,name);
    pointer->rating = rating - '0';
    return pointer;
}

// For separating first line of CSV file which contains book names
int read_first_line(int index, char line[], char bookName[])
{
    int i = index;
    int len = strlen(line);
    int k;
    for(k=0;k<wSize;k++)
    {
        bookName[k]=0;
    }
    int j=0;
    while( i<len-1 && line[i] != ',' && line[i] != NULL && line[i] != ';' && line[i] != '\n' && line[i]!='?')
    {
        bookName[j] = line[i];  // get character by character
        i++;
        j++;
    }
    return i+1;
}

// For reading CSV file line by line and separating users and ratings from lines and creating a table of users
void readCSV(Users *table[])
{
    char line[Size];        // for reading csv file line by line
    char user[wSize];       // for storing user name
    char rating;            // for storing user rating
    char bookNames[Size];   // for storing first line which is book names
    char bookName[wSize];   // for storing book name
    int tableSize = 0;

    FILE *file = fopen("../data/recdataset.csv", "r");
    if ( file == NULL)
    {
        printf("Could not read the file\n");
        return;
    }

    fgets(bookNames, Size, file);   // first line is book names
    while(!feof(file) && fgets(line, Size, file)!=NULL)
    {
        int i=0;
        int bookIndex = 6;      // used for reading book names from bookNames string. book names start from 6. character
        int len = strlen(line);
        int k;
        int number_of_books = 0;

        for(k = 0; k< wSize; k++)  // making sure the array elements are null
        {
            user[k] = 0;
        }

        while(i<len-1)
        {
            rating = '9';   // any value other than 0-5 works

            if( i==0 )
            {
                int j=0;
                while( line[i] != ' ' && line[i] != ',' && line[i] != NULL && line[i] != '\n' && line[i] != ';' && line[i]!='?')
                {
                    user[j] = line[i];  // get character by character
                    i++;
                    j++;
                }
                table[tableSize] = create_user_node(user);
                tableSize++;
            }
            else
            {
                if( line[i] != ' ' && line[i] != ',' && line[i] != NULL && line[i] != ';' && line[i] != '\n' && line[i]!='?')
                {
                    rating = line[i];  // get character by character
                    i++;
                }
                if( line[i] == ';' && line[i-1] == ';')  //CSV dosyasinda bos olan ratinglarda bazi yerlerde ;bosluk; ve bazi yerlerde ;; bosluksuz
                {
                    rating = '0';   // ;; seklinde ise
                }
                if( line[i] == ' ')
                {
                    rating = '0';   // ;bosluk; seklinde ise
                }
                if(rating != '9')
                {
                    bookIndex = read_first_line(bookIndex,bookNames, bookName);
                    table[tableSize-1]->books[number_of_books] = create_book_node(bookName,rating);
                    number_of_books++;
                }
            }
            i++;
        }

        if(line[len-2] == ';')
        {
            rating = '0';
            bookIndex = read_first_line(bookIndex,bookNames, bookName);
            table[tableSize-1]->books[number_of_books] = create_book_node(bookName,rating);
            number_of_books++;
        }

    }
}

// For calculating similarity rate of two users
double pearson_coefficient(Users *userA, Users* userB)
{
    double rA = 0;      //a’nin okudugu kitaplarin puan ortalamasi (ortak kitaplar)
    double rB = 0;      // b’nin okudugu kitaplarin puan ortalamasi (ortak kitaplar)
    int i = 0;
    int number_of_common = 0;   // How many common books exist between two users
    int userA_ratings[20];      // Rating of users A for Common books
    int userB_ratings[20];      // Rating of user B for common books
    double sim;                 // pearson similarity coefficient

    // Only common books between two users are used to calculate average rating. And books with 0 ratings are not included
    while( userA->books[i] != NULL && userB->books[i] != NULL)  // Calculating sum of the ratings
    {
        if(userA->books[i]->rating != 0 && userB->books[i]->rating != 0) // Calculating average of books read by the user
        {
            userA_ratings[number_of_common] = userA->books[i]->rating;
            userB_ratings[number_of_common] = userB->books[i]->rating;
            rA = rA + userA_ratings[number_of_common]; // Sum of the ratings of user A
            rB = rB + userB_ratings[number_of_common]; // Sum of the ratings of user B
            number_of_common++;
        }
        i++;
    }

    rA = rA/number_of_common;   // Mean of ratings of user A
    rB = rB/number_of_common;   // Mean of ratings of user B

    double cov = 0;         // covariance
    double stdDevA = 0;     // standard deviation of user A
    double stdDevB = 0;     // standard deviation of user B

    int j;
    for(j=0;j<number_of_common;j++) // calculating summation of covariance
    {
        cov = cov + (userA_ratings[j] - rA)*(userB_ratings[j] - rB);    // Summation( (Rap - Ra) * (Rbp - Rb) )
        stdDevA = stdDevA + (userA_ratings[j] - rA)*(userA_ratings[j] - rA); // Summation( (Rap - Ra)^2 )
        stdDevB = stdDevB + (userB_ratings[j] - rB)*(userB_ratings[j] - rB);// Summation( (Rbp - Rb) )
    }

    stdDevA = sqrt(stdDevA); // standard deviation of A
    stdDevB = sqrt(stdDevB);// standard deviation of B

    sim = (double) cov/(stdDevA*stdDevB);   // sim(A,B)
    return sim;
}

// For getting index of the New User from users table
int search_for_index(Users* table[], char name[])
{
    int i = 0;
    while(table[i] != NULL)
    {
        if(!strcmp(table[i]->name, name))
        {
            return i;
        }
        i++;
    }
    return -1;
}

// If new pearson coefficient is greater than minimum of K already existing coefficient, then place the new pearson value
void isGreater(Users* table[], int index, double sim, Similars* simls[], int k)
{
    int i;
    int minIndex = 0;
    for(i=1;i<k;i++)
    {
        if(simls[i]->simPoint < simls[minIndex]->simPoint)
        {
            minIndex = i;
        }
    }

    if(simls[minIndex]->simPoint < sim )
    {
        simls[minIndex]->simPoint = sim;
        simls[minIndex]->user = table[index];
    }
}

Similars* create_similars_node(Users *user, double simPoint)
{
    Similars *pointer = (Similars*) malloc (sizeof(Similars));
    pointer->user = user;
    pointer->simPoint = simPoint;
    pointer->mean = 0;
    return pointer;
}

void simlsNullify(Similars *simls[])
{
    int k;
    for(k=0;k<20;k++)
    {
        simls[k] = NULL;
    }
}

// Function for finding K number of similar users to New User
void k_similar(Users *table[], int k, char name[])
{
    Similars *simls[20];            // For storing K users most similar to New User
    int index = search_for_index(table, name);  // save index of New User
    int i;
    simlsNullify(simls);

    for(i=0;i<k;i++)
    {
        double simPoint = pearson_coefficient(table[index], table[i]);
        simls[i] = create_similars_node(table[i], simPoint);   // Create new node and store most similar users
    }

    while(table[i] != NULL) // Calculating pearson coefficient of all users with New User
    {
        if(table[i]->name[0] == 'U')    //  name of normal Users start with 'U' and New Users with 'N'
        {
            double simPoint = pearson_coefficient(table[index],table[i]);
            isGreater(table,i,simPoint,simls,k);    // see if pearson value is greater than existing values
        }
        i++;
    }

    int j;
    for (i = 0; i < k-1; i++)   // Sorting similar users according to pearson values
    {
        int min_idx = i;
        for (j = i+1; j < k; j++)
        {
            if (simls[j]->simPoint > simls[min_idx]->simPoint)
            {
                min_idx = j;
            }
        }
        Similars *pointer = simls[i];
        simls[i] = simls[min_idx];
        simls[min_idx] = pointer;
    }

    printf("<Kullanici Adi, Benzerlik>\n");
    for(i=0;i<k;i++)
    {
        printf("%s, %f\n", simls[i]->user->name, simls[i]->simPoint);
    }

    pred(table,index,simls);
}

void showTable(Users* table[]){
    int i=0;

    while(table[i])
    {
        printf("%s: ",table[i]->name);
        int j=0;
        while(table[i]->books[j] != NULL){
            printf("%s\t",table[i]->books[j]->name);
            j++;
        }
        printf("\n");
        i++;
    }
}

// Function for predicting rating
void pred(Users *table[], int index, Similars *simls[])
{
    int i=0;
    NotRead *notR_books[20];    //For storing Books not read by new user

    for(i=0;i<20;i++)
    {
        notR_books[i] = NULL;
    }

    i = 0;
    double rA = 0;  // For storing average rating of New User

    while(simls[i] != NULL) // Calculating average rating of users most similar to NEW USER
    {
        int k = 0;
        int Number_of_read_books = 0;
        while(simls[i]->user->books[k] != NULL)
        {
            if(simls[i]->user->books[k]->rating != 0)
            {
                simls[i]->mean = simls[i]->mean + simls[i]->user->books[k]->rating; // sum of the rating of the users similar to new user
                Number_of_read_books++;
            }
            k++;
        }
        simls[i]->mean = simls[i]->mean / (Number_of_read_books);  // Average value    (Zero ratings are not included)
        i++;
    }

    i=0;
    int j=0;

    while(table[index]->books[i] != NULL)   // continue until no books are left in the list
    {
        if(table[index]->books[i]->rating == 0)
        {
            notR_books[j] = (NotRead*)malloc(sizeof(NotRead));
            notR_books[j]->notReadBook = table[index]->books[i] ;             // storing list of the books which new user has not read
            notR_books[j]->notReadBookIndex = i;                             // Storing index for easy access
            j++;

        }
        else
        {
            rA = rA + table[index]->books[i]->rating;   // calculating mean or avg rating of the new user
        }
        i++;
    }

    rA = rA/(i-j);  // Average rating of new user, 0 ratings are no included

    j=0;
    while(notR_books[j] != NULL)    // for calculating prediction
    {
        double a = 0;   // summation( sim(a,b)*(Rbp - Rb) )
        double b = 0;   // summation( sim(a,b) )

        notR_books[j]->prediction = rA;
        i=0;
        while(simls[i] != NULL)
        {
            double c = pearson_coefficient(table[index],simls[i]->user);    // sim(a,b)
            b = b+c;    // summation( sim(a,b) )
            a = a + c* (simls[i]->user->books[ notR_books[j]->notReadBookIndex ]->rating -simls[i]->mean); // summation( sim(a,b)*(Rbp - Rb) )
            i++;
        }
        notR_books[j]->prediction = notR_books[j]->prediction + a/b;
        j++;
    }

    i=0;
    printf("Predictions of unread book rating for user %s is as follows:\n", table[index]->name);
    while(notR_books[i] != NULL)
    {
        printf("%d. %s , %f\n",i+1, notR_books[i]->notReadBook->name, notR_books[i]->prediction);
        i++;
    }

    int max_index = 0;
    i=0;
    while(notR_books[i] != NULL)
    {
        if(notR_books[i]->prediction > notR_books[max_index]->prediction)
        {
            max_index = i;
        }
        i++;
    }
    printf("Recommended Book is : %s\n", notR_books[max_index]->notReadBook->name);

}

// For making user entered names uppercase
void toupperCase(char string[]) // converts words to uppercase
{
    int i;
    for (i = 0; string[i]!='\0'; i++) {
        if(string[i] >= 'a' && string[i] <= 'z') {
            string[i] = string[i]-32;
        }
    }
}

void allNewUsers(Users *table[])
{
    int i = 0;
    while(table[i] != NULL)
    {
        if(table[i]->name[0] == 'N')
        {
           k_similar(table, 3, table[i]->name);
        }
        i++;
    }
}

int main()
{
    Users *table[100];
    int i;
    for(i=0;i<100;i++){
        table[i]=NULL;
    }
    readCSV(table);
    char name[20];
    int k;
    int choice;
    printf("Press 1: To Show Data For The User of Your Choice\nPress 2: To Show Data for All New Users\n");
    scanf("%d",&choice);
    if(choice == 1)
    {
        printf("Enter the Name of the NEW USER you want to recommend book( Example: NU1 ):\t");
        scanf("%s",name);

        toupperCase(name);
        printf("Enter the number of similar users you want to find:\t");
        scanf("%d", &k);

        k_similar(table,k,name);
    }
    else {
        allNewUsers(table);
    }
    return 0;
}
