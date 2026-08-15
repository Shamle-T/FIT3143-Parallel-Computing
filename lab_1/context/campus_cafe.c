/******************************************************/
/* Campus Cafe Ordering and Daily Sales System        */
/*                                                    */
/* Topics: loops, constants, conditions, arrays, I/O, */
/* files, functions, pointers and command arguments.  */
/*                                                    */
/******************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ITEMS 5
#define MAX_NAME 50
#define ITEM_NAME_SIZE 30
#define MAX_LINE 150

#define STUDENT_DISCOUNT 0.10
#define SERVICE_TAX 0.06
#define MAX_QUANTITY 10

void clear_input_buffer(void);
void display_main_menu(void);
void display_food_menu(char item_name[][ITEM_NAME_SIZE], double item_price[], int number_of_items);
void reset_quantity(int quantity[], int number_of_items);
void update_quantity(int *quantity, int amount);
int check_order(int quantity[], int number_of_items);
double calculate_subtotal(double item_price[], int quantity[], int number_of_items);
void calculate_total(double subtotal, char student, double *discount, double *tax, double *total);
int calculate_change(double total, double payment, double *change);
void print_receipt(char customer_name[], char student, char item_name[][ITEM_NAME_SIZE],
                   double item_price[], int quantity[], int number_of_items,
                   double subtotal, double discount, double tax, double total,
                   double payment, double change);
void save_transaction(char filename[], char customer_name[], char student,
                      char item_name[][ITEM_NAME_SIZE], double item_price[],
                      int quantity[], int number_of_items, double subtotal,
                      double discount, double tax, double total);
void display_transactions(char filename[]);
void display_sales_summary(char filename[]);
void create_new_order(char filename[], char item_name[][ITEM_NAME_SIZE],
                      double item_price[], int number_of_items);

int main(int argc, char **argv)
{
    int menu_choice;

    char item_name[MAX_ITEMS][ITEM_NAME_SIZE] = {
        "Nasi Lemak",
        "Fried Noodles",
        "Chicken Sandwich",
        "Mineral Water",
        "Iced Milo"};

    double item_price[MAX_ITEMS] = {
        5.50,
        6.00,
        4.50,
        1.50,
        3.00};

    if (argc != 2)
    {
        printf("Usage: %s filename\n", argv[0]);
        return (1);
    }

    menu_choice = 0;

    while (menu_choice != 4)
    {
        display_main_menu();
        printf("Enter your choice: ");

        if (scanf("%d", &menu_choice) != 1)
        {
            printf("Invalid input. Please enter a number.\n");
            clear_input_buffer();
            menu_choice = 0;
            continue;
        }

        switch (menu_choice)
        {
        case 1:
        {
            create_new_order(argv[1], item_name, item_price, MAX_ITEMS);
            break;
        }

        case 2:
        {
            display_transactions(argv[1]);
            break;
        }

        case 3:
        {
            display_sales_summary(argv[1]);
            break;
        }

        case 4:
        {
            printf("\nProgram ended.\n");
            break;
        }

        default:
        {
            printf("Invalid choice. Please enter 1 to 4.\n");
        }
        }
    }

    return (0);
}

void clear_input_buffer(void)
{
    int c;

    c = getchar();
    while (c != '\n' && c != EOF)
    {
        c = getchar();
    }
}

void display_main_menu(void)
{
    printf("\n====================================\n");
    printf("       CAMPUS CAFE SYSTEM\n");
    printf("====================================\n");
    printf("1. Create New Order\n");
    printf("2. View Previous Transactions\n");
    printf("3. Display Daily Sales Summary\n");
    printf("4. Exit\n");
}

void display_food_menu(char item_name[][ITEM_NAME_SIZE], double item_price[], int number_of_items)
{
    int i;

    printf("\n----------- FOOD MENU -----------\n");

    for (i = 0; i < number_of_items; i++)
    {
        printf("%d. %-20s RM%.2f\n", i + 1, item_name[i], item_price[i]);
    }

    printf("0. Finish Order\n");
    printf("---------------------------------\n");
}

void reset_quantity(int quantity[], int number_of_items)
{
    int i;

    for (i = 0; i < number_of_items; i++)
    {
        quantity[i] = 0;
    }
}

void update_quantity(int *quantity, int amount)
{
    *quantity = *quantity + amount;
}

int check_order(int quantity[], int number_of_items)
{
    int i;

    for (i = 0; i < number_of_items; i++)
    {
        if (quantity[i] > 0)
        {
            return (1);
        }
    }

    return (0);
}

double calculate_subtotal(double item_price[], int quantity[], int number_of_items)
{
    int i;
    double subtotal;

    subtotal = 0.0;

    for (i = 0; i < number_of_items; i++)
    {
        subtotal = subtotal + item_price[i] * quantity[i];
    }

    return (subtotal);
}

void calculate_total(double subtotal, char student, double *discount, double *tax, double *total)
{
    double amount_after_discount;

    if (student == 'Y' || student == 'y')
    {
        *discount = subtotal * STUDENT_DISCOUNT;
    }
    else
    {
        *discount = 0.0;
    }

    amount_after_discount = subtotal - *discount;
    *tax = amount_after_discount * SERVICE_TAX;
    *total = amount_after_discount + *tax;
}

int calculate_change(double total, double payment, double *change)
{
    if (payment >= total)
    {
        *change = payment - total;
        return (1);
    }

    return (0);
}

void create_new_order(char filename[], char item_name[][ITEM_NAME_SIZE],
                      double item_price[], int number_of_items)
{
    char customer_name[MAX_NAME];
    char student;
    char *newline;

    int quantity[MAX_ITEMS];
    int item_code;
    int amount;
    int order_finished;
    int payment_accepted;

    double subtotal;
    double discount;
    double tax;
    double total;
    double payment;
    double change;

    reset_quantity(quantity, number_of_items);
    clear_input_buffer();

    printf("\nEnter customer name: ");
    fgets(customer_name, MAX_NAME, stdin);

    newline = strchr(customer_name, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
    }

    while (strlen(customer_name) == 0)
    {
        printf("Customer name cannot be empty. Enter again: ");
        fgets(customer_name, MAX_NAME, stdin);
        newline = strchr(customer_name, '\n');
        if (newline != NULL)
        {
            *newline = '\0';
        }
    }

    student = ' ';

    while (student != 'Y' && student != 'y' && student != 'N' && student != 'n')
    {
        printf("Is the customer a student? (Y/N): ");
        scanf(" %c", &student);

        if (student != 'Y' && student != 'y' && student != 'N' && student != 'n')
        {
            printf("Invalid input. Please enter Y or N.\n");
        }
    }

    order_finished = 0;

    while (order_finished == 0)
    {
        display_food_menu(item_name, item_price, number_of_items);
        printf("Enter item code: ");

        if (scanf("%d", &item_code) != 1)
        {
            printf("Invalid input. Please enter a number.\n");
            clear_input_buffer();
            continue;
        }

        if (item_code == 0)
        {
            if (check_order(quantity, number_of_items) == 1)
            {
                order_finished = 1;
            }
            else
            {
                printf("Please order at least one item.\n");
            }
        }
        else if (item_code < 1 || item_code > number_of_items)
        {
            printf("Invalid item code.\n");
        }
        else
        {
            printf("Enter quantity: ");

            if (scanf("%d", &amount) != 1)
            {
                printf("Invalid input. Please enter a number.\n");
                clear_input_buffer();
                continue;
            }

            if (amount < 1 || amount > MAX_QUANTITY)
            {
                printf("Quantity must be between 1 and %d.\n", MAX_QUANTITY);
            }
            else
            {
                update_quantity(&quantity[item_code - 1], amount);
                printf("Order updated.\n");
            }
        }
    }

    subtotal = calculate_subtotal(item_price, quantity, number_of_items);

    calculate_total(
        subtotal,
        student,
        &discount,
        &tax,
        &total);

    payment_accepted = 0;
    payment = 0.0;
    change = 0.0;

    while (payment_accepted == 0)
    {
        printf("\nFinal total: RM%.2f\n", total);
        printf("Enter payment amount: RM");

        if (scanf("%lf", &payment) != 1)
        {
            printf("Invalid payment input.\n");
            clear_input_buffer();
            continue;
        }

        if (payment < 0.0)
        {
            printf("Payment cannot be negative.\n");
        }
        else
        {
            payment_accepted = calculate_change(total, payment, &change);

            if (payment_accepted == 0)
            {
                printf("Insufficient payment. Please enter at least RM%.2f.\n", total);
            }
        }
    }

    print_receipt(
        customer_name,
        student,
        item_name,
        item_price,
        quantity,
        number_of_items,
        subtotal,
        discount,
        tax,
        total,
        payment,
        change);

    save_transaction(
        filename,
        customer_name,
        student,
        item_name,
        item_price,
        quantity,
        number_of_items,
        subtotal,
        discount,
        tax,
        total);
}

void print_receipt(char customer_name[], char student,
                   char item_name[][ITEM_NAME_SIZE], double item_price[],
                   int quantity[], int number_of_items,
                   double subtotal, double discount, double tax,
                   double total, double payment, double change)
{
    int i;
    double item_total;

    printf("\n=============================================\n");
    printf("             CAMPUS CAFE RECEIPT\n");
    printf("=============================================\n");
    printf("Customer: %s\n", customer_name);

    if (student == 'Y' || student == 'y')
    {
        printf("Student: Yes\n");
    }
    else
    {
        printf("Student: No\n");
    }

    printf("\n%-22s %5s %10s\n", "Item", "Qty", "Amount");
    printf("---------------------------------------------\n");

    for (i = 0; i < number_of_items; i++)
    {
        if (quantity[i] > 0)
        {
            item_total = item_price[i] * quantity[i];
            printf("%-22s %5d %10.2f\n", item_name[i], quantity[i], item_total);
        }
    }

    printf("---------------------------------------------\n");
    printf("%-29s RM%10.2f\n", "Subtotal:", subtotal);
    printf("%-29s RM%10.2f\n", "Discount:", discount);
    printf("%-29s RM%10.2f\n", "Service tax:", tax);
    printf("%-29s RM%10.2f\n", "Final total:", total);
    printf("%-29s RM%10.2f\n", "Payment:", payment);
    printf("%-29s RM%10.2f\n", "Change:", change);
    printf("=============================================\n");
    printf("Thank you!\n");
}

void save_transaction(char filename[], char customer_name[], char student,
                      char item_name[][ITEM_NAME_SIZE], double item_price[],
                      int quantity[], int number_of_items, double subtotal,
                      double discount, double tax, double total)
{
    FILE *fp;
    int i;
    int total_units;
    double item_total;

    fp = fopen(filename, "a");

    if (fp == NULL)
    {
        printf("Error: Unable to open sales file.\n");
        return;
    }

    fprintf(fp, "CUSTOMER|%s\n", customer_name);
    fprintf(fp, "STUDENT|%c\n", student);

    total_units = 0;

    for (i = 0; i < number_of_items; i++)
    {
        if (quantity[i] > 0)
        {
            item_total = item_price[i] * quantity[i];
            total_units = total_units + quantity[i];
            fprintf(fp, "ITEM|%s|%d|%.2f\n",
                    item_name[i], quantity[i], item_total);
        }
    }

    fprintf(fp, "UNITS|%d\n", total_units);
    fprintf(fp, "SUBTOTAL|%.2f\n", subtotal);
    fprintf(fp, "DISCOUNT|%.2f\n", discount);
    fprintf(fp, "TAX|%.2f\n", tax);
    fprintf(fp, "TOTAL|%.2f\n", total);
    fprintf(fp, "END\n\n");

    fclose(fp);

    printf("Transaction saved to %s.\n", filename);
}

void display_transactions(char filename[])
{
    FILE *fp;
    int c;

    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("No previous transaction file was found.\n");
        return;
    }

    printf("\n========== PREVIOUS TRANSACTIONS ==========\n");

    c = fgetc(fp);

    while (c != EOF)
    {
        putchar(c);
        c = fgetc(fp);
    }

    printf("===========================================\n");

    fclose(fp);
}

void display_sales_summary(char filename[])
{
    FILE *fp;
    char line[MAX_LINE];

    int value;
    int transaction_count;
    int total_units;

    double amount;
    double total_revenue;
    double highest_transaction;
    double average_transaction;

    transaction_count = 0;
    total_units = 0;
    total_revenue = 0.0;
    highest_transaction = 0.0;
    average_transaction = 0.0;

    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("No sales data were found.\n");
        return;
    }

    while (fgets(line, MAX_LINE, fp) != NULL)
    {
        if (sscanf(line, "UNITS|%d", &value) == 1)
        {
            total_units = total_units + value;
        }
        else if (sscanf(line, "TOTAL|%lf", &amount) == 1)
        {
            total_revenue = total_revenue + amount;
            transaction_count = transaction_count + 1;

            if (amount > highest_transaction)
            {
                highest_transaction = amount;
            }
        }
    }

    if (transaction_count > 0)
    {
        average_transaction = total_revenue / transaction_count;
    }

    printf("\n====================================\n");
    printf("         DAILY SALES SUMMARY\n");
    printf("====================================\n");
    printf("Number of transactions: %d\n", transaction_count);
    printf("Total units sold:       %d\n", total_units);
    printf("Total revenue:          RM%.2f\n", total_revenue);
    printf("Highest transaction:    RM%.2f\n", highest_transaction);
    printf("Average transaction:    RM%.2f\n", average_transaction);
    printf("====================================\n");

    fclose(fp);
}
