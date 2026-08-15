/******************************************************/
/* Campus Cafe Ordering and Daily Sales System        */
/*                                                    */
/* Students must complete the missing sections.       */

/* Run code:                                          */
/* gcc campus_cafe.c -o campus_cafe                   */
/* ./campus_cafe sales.txt                            */ 
/******************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Symbolic constants */

#define MAX_ITEMS 5
#define MAX_NAME 50
#define ITEM_NAME_SIZE 30
#define MAX_LINE 100

#define STUDENT_DISCOUNT 0.10
#define SERVICE_TAX 0.06
#define MAX_QUANTITY 10


/* Function prototypes */

void display_main_menu(void);

void display_food_menu(
    char item_name[][ITEM_NAME_SIZE],
    double item_price[],
    int number_of_items
);

void reset_quantity(
    int quantity[],
    int number_of_items
);

void update_quantity(
    int *quantity,
    int amount
);

int check_order(
    int quantity[],
    int number_of_items
);

double calculate_subtotal(
    double item_price[],
    int quantity[],
    int number_of_items
);

void calculate_total(
    double subtotal,
    char student,
    double *discount,
    double *tax,
    double *total
);

int calculate_change(
    double total,
    double payment,
    double *change
);

void print_receipt(
    char customer_name[],
    char student,
    char item_name[][ITEM_NAME_SIZE],
    double item_price[],
    int quantity[],
    int number_of_items,
    double subtotal,
    double discount,
    double tax,
    double total,
    double payment,
    double change
);

void save_transaction(
    char filename[],
    char customer_name[],
    char student,
    char item_name[][ITEM_NAME_SIZE],
    double item_price[],
    int quantity[],
    int number_of_items,
    double subtotal,
    double discount,
    double tax,
    double total
);

void display_transactions(
    char filename[]
);

void display_sales_summary(
    char filename[]
);

void create_new_order(
    char filename[],
    char item_name[][ITEM_NAME_SIZE],
    double item_price[],
    int number_of_items
);


/******************************************************/
/* Main function                                      */
/******************************************************/

int main(int argc, char** argv)
{
    int menu_choice;

    char item_name[MAX_ITEMS][ITEM_NAME_SIZE] = {
        "Nasi Lemak",
        "Fried Noodles",
        "Chicken Sandwich",
        "Mineral Water",
        "Iced Milo"
    };

    double item_price[MAX_ITEMS] = {
        5.50,
        6.00,
        4.50,
        1.50,
        3.00
    };

    /*
     * argc gives the number of command-line strings.
     *
     * argv[0] is the program name.
     * argv[1] should be the sales filename.
     *
     * Example:
     * ./cafe sales.txt
     */

    if (argc != 2)
    {
        printf("Usage: %s filename\n", argv[0]);
        return(1);
    }

    menu_choice = 0;

    while (menu_choice != 4)
    {
        display_main_menu();

        printf("Enter your choice: ");
        scanf("%d", &menu_choice);

        switch (menu_choice)
        {
            case 1:
            {
                /*
                 * Call create_new_order.
                 *
                 * Pass:
                 * argv[1]
                 * item_name
                 * item_price
                 * MAX_ITEMS
                 */

                break;
            }

            case 2:
            {
                /*
                 * Call display_transactions.
                 *
                 * The filename is stored in argv[1].
                 */

                break;
            }

            case 3:
            {
                /*
                 * Call display_sales_summary.
                 */

                break;
            }

            case 4:
            {
                printf("\nProgram ended.\n");
                break;
            }

            default:
            {
                /*
                 * Display an invalid-choice message.
                 */

                break;
            }
        }
    }

    return(0);
}


/******************************************************/
/* Display the main menu                              */
/******************************************************/

void display_main_menu(void)
{
    /*
     * Display:
     *
     * ====================================
     *        CAMPUS CAFE SYSTEM
     * ====================================
     * 1. Create New Order
     * 2. View Previous Transactions
     * 3. Display Daily Sales Summary
     * 4. Exit
     */
}


/******************************************************/
/* Display all cafe items                             */
/******************************************************/

void display_food_menu(
    char item_name[][ITEM_NAME_SIZE],
    double item_price[],
    int number_of_items
)
{
    int i;

    /*
     * Print the food-menu heading.
     *
     * Use a for loop to display every item.
     *
     * The array index begins at 0.
     * The item code shown to the customer begins at 1.
     *
     * Therefore, display i + 1 as the item code.
     *
     * Also display:
     * 0. Finish Order
     */

    for (i = 0; i < number_of_items; i++)
    {
        /*
         * Display:
         * item code
         * item name
         * item price
         */
    }
}


/******************************************************/
/* Reset every item quantity to zero                  */
/******************************************************/

void reset_quantity(
    int quantity[],
    int number_of_items
)
{
    int i;

    /*
     * Use a for loop.
     *
     * Set every element in quantity[] to zero.
     */

    for (i = 0; i < number_of_items; i++)
    {
        /*
         * Reset the current quantity.
         */
    }
}


/******************************************************/
/* Update one quantity using a pointer                */
/******************************************************/

void update_quantity(
    int *quantity,
    int amount
)
{
    /*
     * quantity contains the address of one element
     * from the quantity array.
     *
     * Use the dereference operator * to change the
     * value stored at that address.
     *
     * Add amount to the original quantity.
     *
     * This is similar to the exchange function shown
     * in the Week 2 exercise.
     */
}


/******************************************************/
/* Check whether at least one item was ordered        */
/******************************************************/

int check_order(
    int quantity[],
    int number_of_items
)
{
    int i;

    /*
     * Use a loop to examine every quantity.
     *
     * If any quantity is greater than zero,
     * return 1.
     *
     * If no item was ordered, return 0.
     */

    for (i = 0; i < number_of_items; i++)
    {
        /*
         * Check the current quantity.
         */
    }

    return(0);
}


/******************************************************/
/* Calculate the subtotal                             */
/******************************************************/

double calculate_subtotal(
    double item_price[],
    int quantity[],
    int number_of_items
)
{
    int i;
    double subtotal;

    subtotal = 0.0;

    /*
     * Use a loop.
     *
     * For each item:
     *
     * amount = item price multiplied by quantity
     *
     * Add the amount to subtotal.
     */

    for (i = 0; i < number_of_items; i++)
    {
        /*
         * Add the amount for the current item.
         */
    }

    return(subtotal);
}


/******************************************************/
/* Calculate discount, tax and final total            */
/******************************************************/

void calculate_total(
    double subtotal,
    char student,
    double *discount,
    double *tax,
    double *total
)
{
    double amount_after_discount;

    /*
     * discount, tax and total are pointer variables.
     *
     * They contain the addresses of variables from the
     * calling function.
     *
     * Use *discount, *tax and *total when assigning values.
     */

    /*
     * If student is 'Y' or 'y':
     *
     * discount = subtotal multiplied by STUDENT_DISCOUNT
     *
     * Otherwise:
     *
     * discount = 0
     */


    /*
     * Calculate amount_after_discount.
     */


    /*
     * Calculate the service tax.
     *
     * tax = amount_after_discount multiplied by SERVICE_TAX
     */


    /*
     * Calculate the final total.
     *
     * total = amount_after_discount plus tax
     */
}


/******************************************************/
/* Check payment and calculate change                 */
/******************************************************/

int calculate_change(
    double total,
    double payment,
    double *change
)
{
    /*
     * If payment is greater than or equal to total:
     *
     * 1. Calculate the change using *change.
     * 2. Return 1.
     *
     * Otherwise:
     *
     * Return 0.
     */

    return(0);
}


/******************************************************/
/* Create a new customer order                        */
/******************************************************/

void create_new_order(
    char filename[],
    char item_name[][ITEM_NAME_SIZE],
    double item_price[],
    int number_of_items
)
{
    char customer_name[MAX_NAME];
    char student;

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

    /*
     * Reset all quantities before accepting a new order.
     */


    /*
     * scanf leaves a newline character in the input.
     *
     * Use getchar() before fgets() to remove that newline.
     */


    /*
     * Ask for the customer name.
     *
     * Use fgets() because the name may contain spaces.
     */


    /*
     * fgets may store '\n' inside customer_name.
     *
     * Use strchr() to find the newline character.
     * If found, replace it with '\0'.
     *
     * Students may use a character pointer:
     *
     * char *newline;
     *
     * newline = strchr(customer_name, '\n');
     */


    /*
     * Ask:
     * Is the customer a student? (Y/N)
     *
     * Use scanf(" %c", &student);
     *
     * The space before %c skips previous whitespace.
     */


    /*
     * Start the ordering loop.
     */

    order_finished = 0;

    while (order_finished == 0)
    {
        /*
         * Display the food menu.
         */


        /*
         * Ask for the item code.
         */


        if (item_code == 0)
        {
            /*
             * Call check_order().
             *
             * If at least one item was ordered,
             * set order_finished to 1.
             *
             * Otherwise, display a message and continue.
             */
        }
        else if (
            item_code < 1 ||
            item_code > number_of_items
        )
        {
            /*
             * Display an invalid-item-code message.
             */
        }
        else
        {
            /*
             * Ask for the quantity.
             */


            if (
                amount < 1 ||
                amount > MAX_QUANTITY
            )
            {
                /*
                 * Display an invalid-quantity message.
                 */
            }
            else
            {
                /*
                 * Call update_quantity().
                 *
                 * The customer item code begins at 1,
                 * but the array index begins at 0.
                 *
                 * Pass the address of the correct
                 * quantity array element.
                 *
                 * Example form:
                 *
                 * update_quantity(
                 *     &quantity[item_code - 1],
                 *     amount
                 * );
                 */
            }
        }
    }


    /*
     * Calculate the subtotal by calling
     * calculate_subtotal().
     */


    /*
     * Calculate discount, tax and total by calling
     * calculate_total().
     *
     * Pass:
     * &discount
     * &tax
     * &total
     *
     * These are the addresses of the original variables.
     */


    /*
     * Ask for payment until sufficient payment is entered.
     */

    payment_accepted = 0;

    while (payment_accepted == 0)
    {
        /*
         * Display the total.
         * Ask for the payment.
         */


        /*
         * Call calculate_change().
         *
         * Pass &change because the function needs to change
         * the original change variable.
         */


        /*
         * If calculate_change returns 0,
         * display an insufficient-payment message.
         *
         * If it returns 1,
         * set payment_accepted to 1.
         */
    }


    /*
     * Call print_receipt().
     */


    /*
     * Call save_transaction().
     */
}


/******************************************************/
/* Print the customer receipt                         */
/******************************************************/

void print_receipt(
    char customer_name[],
    char student,
    char item_name[][ITEM_NAME_SIZE],
    double item_price[],
    int quantity[],
    int number_of_items,
    double subtotal,
    double discount,
    double tax,
    double total,
    double payment,
    double change
)
{
    int i;
    double item_total;

    /*
     * Display the receipt heading.
     *
     * Display the customer name and student status.
     *
     * Display column headings:
     *
     * Item
     * Quantity
     * Amount
     */


    for (i = 0; i < number_of_items; i++)
    {
        if (quantity[i] > 0)
        {
            /*
             * Calculate the total for the current item.
             *
             * item_total =
             * item_price[i] multiplied by quantity[i]
             */


            /*
             * Display:
             * item name
             * quantity
             * item total
             */
        }
    }


    /*
     * Display:
     * subtotal
     * discount
     * service tax
     * final total
     * payment
     * change
     */
}


/******************************************************/
/* Save one transaction to a file                     */
/******************************************************/

void save_transaction(
    char filename[],
    char customer_name[],
    char student,
    char item_name[][ITEM_NAME_SIZE],
    double item_price[],
    int quantity[],
    int number_of_items,
    double subtotal,
    double discount,
    double tax,
    double total
)
{
    FILE *fp;
    int i;
    int total_units;
    double item_total;

    /*
     * Open the file using append mode.
     *
     * Example:
     *
     * fp = fopen(filename, "a");
     */

    fp = NULL;


    /*
     * Check whether fp is equal to NULL.
     *
     * If the file cannot be opened:
     * display an error message and return.
     */

    if (fp == NULL)
    {
        return;
    }


    /*
     * Write the customer name and student status.
     *
     * Suggested format:
     *
     * CUSTOMER|Ali Ahmad
     * STUDENT|Y
     */


    total_units = 0;

    for (i = 0; i < number_of_items; i++)
    {
        if (quantity[i] > 0)
        {
            /*
             * Calculate item_total.
             *
             * Add the quantity to total_units.
             *
             * Write the item information using fprintf().
             *
             * Suggested format:
             *
             * ITEM|Nasi Lemak|2|11.00
             */
        }
    }


    /*
     * Write:
     *
     * UNITS|total units
     * SUBTOTAL|subtotal
     * DISCOUNT|discount
     * TAX|tax
     * TOTAL|total
     * END
     */


    /*
     * Close the file using fclose().
     */
}


/******************************************************/
/* Display every character from the transaction file */
/******************************************************/

void display_transactions(
    char filename[]
)
{
    FILE *fp;
    int c;

    /*
     * Open the file using read mode.
     */

    fp = NULL;


    if (fp == NULL)
    {
        printf("No previous transaction file was found.\n");
        return;
    }


    /*
     * Use fgetc() to read one character at a time.
     *
     * Use putchar() to display the character.
     *
     * Continue until EOF is reached.
     *
     * This should be similar to the character I/O example
     * from the Week 2 exercise.
     */

    c = 0;


    /*
     * Close the file.
     */
}


/******************************************************/
/* Read the file and display the sales summary        */
/******************************************************/

void display_sales_summary(
    char filename[]
)
{
    FILE *fp;

    char word[MAX_NAME];

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


    /*
     * Open the file using read mode.
     */

    fp = NULL;


    if (fp == NULL)
    {
        printf("No sales data were found.\n");
        return;
    }


    /*
     * Read the file using fscanf().
     *
     * One simple method is to read the first word before
     * the | symbol.
     *
     * Look for:
     *
     * UNITS|number
     * TOTAL|amount
     *
     * When UNITS is found:
     * add the number to total_units.
     *
     * When TOTAL is found:
     * 1. Add the amount to total_revenue.
     * 2. Increase transaction_count.
     * 3. Compare the amount with highest_transaction.
     *
     * You may also use fgets() and sscanf() if preferred.
     */


    /*
     * Calculate the average only when transaction_count
     * is greater than zero.
     */


    /*
     * Display:
     *
     * Number of transactions
     * Total units sold
     * Total revenue
     * Highest transaction
     * Average transaction
     */


    /*
     * Close the file.
     */


    /*
     * These statements may be removed after the function
     * has been completed.
     */

    (void)word;
    (void)value;
    (void)amount;
    (void)transaction_count;
    (void)total_units;
    (void)total_revenue;
    (void)highest_transaction;
    (void)average_transaction;
}