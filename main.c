#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MENU 10
#define MAX_ITEMS_PER_ORDER 5
#define MAX_ORDERS 100

// Define Order Status
typedef enum {
    PENDING,
    PREPARING,
    READY,
    SERVED,
    CANCELLED
} OrderStatus;

// Structure for Menu Item
typedef struct {
    int id;
    char name[30];
    float price;
} MenuItem;

// Structure for an item within an order
typedef struct {
    int itemId;
    int quantity;
} OrderItem;

// Structure for an Order
typedef struct {
    int orderId;
    int tableNumber;
    OrderItem items[MAX_ITEMS_PER_ORDER];
    int itemCount;
    OrderStatus status;
    float totalAmount;
    int hour; // Recorded order hour (0-23) for busiest hours report
} Order;

// Queue Node for Kitchen Queue (Implemented using Pointers / Linked List)
typedef struct QueueNode {
    int orderId;
    struct QueueNode* next;
} QueueNode;

// Queue structure
typedef struct {
    QueueNode* front;
    QueueNode* rear;
} KitchenQueue;

// Global Menu Dataset
MenuItem menu[MAX_MENU] = {
    {1, "Margherita Pizza", 12.99},
    {2, "Pepperoni Pizza", 14.99},
    {3, "Cheeseburger", 8.50},
    {4, "Veggie Burger", 7.50},
    {5, "Caesar Salad", 6.00},
    {6, "French Fries", 3.50},
    {7, "Garlic Bread", 4.50},
    {8, "Coca Cola", 2.50},
    {9, "Iced Tea", 2.75},
    {10, "Chocolate Brownie", 5.00}
};

Order orders[MAX_ORDERS];
int orderCount = 0;
int nextOrderId = 101;

// Function Prototypes
void displayMenu();
void placeOrder(KitchenQueue* q);
void viewActiveOrders();
void updateOrderStatus(KitchenQueue* q);
void cancelOrder();
void generateBill();
void generateReports();

// Queue Operations
void initQueue(KitchenQueue* q);
void enqueue(KitchenQueue* q, int orderId);
int dequeue(KitchenQueue* q);
int isQueueEmpty(KitchenQueue* q);
void displayQueue(KitchenQueue* q);

// Helper function to convert enum status to readable string
const char* getStatusString(OrderStatus status) {
    switch (status) {
        case PENDING: return "Pending";
        case PREPARING: return "Preparing";
        case READY: return "Ready";
        case SERVED: return "Served";
        case CANCELLED: return "Cancelled";
        default: return "Unknown";
    }
}

int main() {
    KitchenQueue kitchenQueue;
    initQueue(&kitchenQueue);
    
    int choice;
    do {
        printf("\n=========================================\n");
        printf("    RESTAURANT ORDER MANAGEMENT SYSTEM   \n");
        printf("=========================================\n");
        printf("1. Display Menu\n");
        printf("2. Place New Order\n");
        printf("3. View Active Orders & Kitchen Queue\n");
        printf("4. Update Order Status (Kitchen)\n");
        printf("5. Cancel Order\n");
        printf("6. Generate Bill & Invoice\n");
        printf("7. View Analytics & Reports\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while(getchar() != '\n'); // Clear input buffer
            continue;
        }

        switch (choice) {
            case 1:
                displayMenu();
                break;
            case 2:
                placeOrder(&kitchenQueue);
                break;
            case 3:
                viewActiveOrders();
                displayQueue(&kitchenQueue);
                break;
            case 4:
                updateOrderStatus(&kitchenQueue);
                break;
            case 5:
                cancelOrder();
                break;
            case 6:
                generateBill();
                break;
            case 7:
                generateReports();
                break;
            case 8:
                printf("\nExiting system. Thank you!\n");
                break;
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
    } while (choice != 8);

    return 0;
}

// Queue Initialization
void initQueue(KitchenQueue* q) {
    q->front = NULL;
    q->rear = NULL;
}

// Check if Queue is Empty
int isQueueEmpty(KitchenQueue* q) {
    return (q->front == NULL);
}

// Enqueue operation using dynamic memory allocation (pointers)
void enqueue(KitchenQueue* q, int orderId) {
    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (!newNode) {
        printf("Memory allocation failed!\n");
        return;
    }
    newNode->orderId = orderId;
    newNode->next = NULL;
    
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}

// Dequeue operation
int dequeue(KitchenQueue* q) {
    if (isQueueEmpty(q)) {
        return -1;
    }
    QueueNode* temp = q->front;
    int orderId = temp->orderId;
    q->front = q->front->next;
    
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    return orderId;
}

// Display Kitchen Queue
void displayQueue(KitchenQueue* q) {
    printf("\n--- Kitchen Queue (FIFO) ---\n");
    if (isQueueEmpty(q)) {
        printf("Kitchen queue is currently empty.\n");
        return;
    }
    QueueNode* current = q->front;
    printf("Queue Order: ");
    while (current != NULL) {
        printf("[Order #%d] -> ", current->orderId);
        current = current->next;
    }
    printf("NULL\n");
}

// Display Menu
void displayMenu() {
    printf("\n================ MENU ================\n");
    printf("%-5s %-25s %-10s\n", "ID", "Item Name", "Price ($)");
    printf("--------------------------------------\n");
    for (int i = 0; i < MAX_MENU; i++) {
        printf("%-5d %-25s $%-9.2f\n", menu[i].id, menu[i].name, menu[i].price);
    }
    printf("======================================\n");
}

// Place Order
void placeOrder(KitchenQueue* q) {
    if (orderCount >= MAX_ORDERS) {
        printf("\nOrder limit reached! Cannot take more orders.\n");
        return;
    }

    Order newOrder;
    newOrder.orderId = nextOrderId++;
    newOrder.status = PENDING;
    newOrder.totalAmount = 0.0;
    
    // Capture current hour for busiest hour report
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    newOrder.hour = tm.tm_hour;

    printf("\nEnter Table Number: ");
    scanf("%d", &newOrder.tableNumber);

    displayMenu();
    printf("Enter number of distinct items to order (Max %d): ", MAX_ITEMS_PER_ORDER);
    scanf("%d", &newOrder.itemCount);

    if (newOrder.itemCount <= 0 || newOrder.itemCount > MAX_ITEMS_PER_ORDER) {
        printf("Invalid item count. Order cancelled.\n");
        nextOrderId--;
        return;
    }

    for (int i = 0; i < newOrder.itemCount; i++) {
        int id, qty, found = 0;
        printf("Enter Item ID for item %d: ", i + 1);
        scanf("%d", &id);

        for (int j = 0; j < MAX_MENU; j++) {
            if (menu[j].id == id) {
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Invalid Item ID! Try again.\n");
            i--; // Retry item selection
            continue;
        }

        printf("Enter Quantity for Item ID %d: ", id);
        scanf("%d", &qty);
        if (qty <= 0) qty = 1;

        newOrder.items[i].itemId = id;
        newOrder.items[i].quantity = qty;
        
        // Calculate subtotal price
        for (int j = 0; j < MAX_MENU; j++) {
            if (menu[j].id == id) {
                newOrder.totalAmount += menu[j].price * qty;
            }
        }
    }

    orders[orderCount++] = newOrder;
    enqueue(q, newOrder.orderId);

    printf("\n[SUCCESS] Order #%d placed successfully for Table #%d! Added to Kitchen Queue.\n", 
           newOrder.orderId, newOrder.tableNumber);
}

// View Active Orders
void viewActiveOrders() {
    printf("\n================ ACTIVE ORDERS ================\n");
    printf("%-9s %-7s %-12s %-12s %-10s\n", "Order ID", "Table", "Status", "Total ($)", "Items");
    printf("-----------------------------------------------\n");
    
    int activeCount = 0;
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].status != CANCELLED && orders[i].status != SERVED) {
            activeCount++;
            printf("%-9d %-7d %-12s $%-11.2f %-10d\n", 
                   orders[i].orderId, 
                   orders[i].tableNumber, 
                   getStatusString(orders[i].status), 
                   orders[i].totalAmount, 
                   orders[i].itemCount);
        }
    }
    if (activeCount == 0) {
        printf("No active orders currently.\n");
    }
    printf("==============================================-\n");
}

// Update Order Status
void updateOrderStatus(KitchenQueue* q) {
    if (isQueueEmpty(q)) {
        printf("\nNo orders in the kitchen queue to process.\n");
        return;
    }

    displayQueue(q);

    int targetId;
    printf("Enter Order ID to update status: ");
    scanf("%d", &targetId);

    int foundIdx = -1;
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].orderId == targetId) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx == -1) {
        printf("Order ID not found!\n");
        return;
    }

    printf("Current Status: %s\n", getStatusString(orders[foundIdx].status));
    printf("Select new status:\n");
    printf("1. Preparing\n");
    printf("2. Ready\n");
    printf("3. Served\n");
    printf("Enter choice: ");
    int choice;
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            orders[foundIdx].status = PREPARING;
            printf("Order #%d status updated to PREPARING.\n", targetId);
            break;
        case 2:
            orders[foundIdx].status = READY;
            printf("Order #%d status updated to READY.\n", targetId);
            break;
        case 3:
            orders[foundIdx].status = SERVED;
            printf("Order #%d status updated to SERVED.\n", targetId);
            // Optionally dequeue when served/processed
            break;
        default:
            printf("Invalid status choice!\n");
    }
}

// Cancel Order
void cancelOrder() {
    int targetId;
    printf("\nEnter Order ID to cancel: ");
    scanf("%d", &targetId);

    int foundIdx = -1;
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].orderId == targetId) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx == -1) {
        printf("Order ID not found!\n");
        return;
    }

    if (orders[foundIdx].status == SERVED) {
        printf("Cannot cancel an order that has already been served!\n");
        return;
    }

    orders[foundIdx].status = CANCELLED;
    printf("[SUCCESS] Order #%d has been cancelled.\n", targetId);
}

// Generate Bill & Itemized Invoice
void generateBill() {
    int targetId;
    printf("\nEnter Order ID to generate bill: ");
    scanf("%d", &targetId);

    int foundIdx = -1;
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].orderId == targetId) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx == -1 || orders[foundIdx].status == CANCELLED) {
        printf("Valid Order ID not found or order was cancelled!\n");
        return;
    }

    Order o = orders[foundIdx];
    float subtotal = o.totalAmount;
    float tax = subtotal * 0.08;           // 8% Tax
    float serviceCharge = subtotal * 0.10; // 10% Service Charge
    float grandTotal = subtotal + tax + serviceCharge;

    printf("\n=========================================\n");
    printf("          RESTAURANT INVOICE             \n");
    printf("=========================================\n");
    printf(" Order ID     : #%d\n", o.orderId);
    printf(" Table Number : %d\n", o.tableNumber);
    printf(" Status       : %s\n", getStatusString(o.status));
    printf("-----------------------------------------\n");
    printf(" %-20s %-5s %-10s\n", "Item", "Qty", "Price ($)");
    printf("-----------------------------------------\n");

    for (int i = 0; i < o.itemCount; i++) {
        char itemName[30];
        float itemPrice = 0;
        for (int j = 0; j < MAX_MENU; j++) {
            if (menu[j].id == o.items[i].itemId) {
                strcpy(itemName, menu[j].name);
                itemPrice = menu[j].price;
                break;
            }
        }
        printf(" %-20s %-5d $%-9.2f\n", itemName, o.items[i].quantity, itemPrice * o.items[i].quantity);
    }

    printf("-----------------------------------------\n");
    printf(" Subtotal        : $%.2f\n", subtotal);
    printf(" Tax (8%)        : $%.2f\n", tax);
    printf(" Service Charge  : $%.2f\n", serviceCharge);
    printf("=========================================\n");
    printf(" GRAND TOTAL     : $%.2f\n", grandTotal);
    printf("=========================================\n");
}

// Reports & Analytics
void generateReports() {
    printf("\n=========================================\n");
    printf("          ANALYTICS & REPORTS            \n");
    printf("=========================================\n");
    
    // 1. Daily Revenue Computation
    float totalRevenue = 0;
    int completedOrders = 0;
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].status != CANCELLED) {
            totalRevenue += orders[i].totalAmount;
            completedOrders++;
        }
    }
    printf("1. Total Active Revenue : $%.2f (%d valid orders)\n", totalRevenue, completedOrders);

    // 2. Frequency Analysis & Sorting for Most-Ordered Items
    int itemFrequencies[MAX_MENU + 1] = {0};
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].status != CANCELLED) {
            for (int j = 0; j < orders[i].itemCount; j++) {
                int id = orders[i].items[j].itemId;
                int qty = orders[i].items[j].quantity;
                itemFrequencies[id] += qty;
            }
        }
    }

    typedef struct {
        int itemId;
        int totalQty;
    } ItemFreq;

    ItemFreq freqList[MAX_MENU];
    for (int i = 0; i < MAX_MENU; i++) {
        freqList[i].itemId = menu[i].id;
        freqList[i].totalQty = itemFrequencies[menu[i].id];
    }

    // Bubble Sort to sort items in descending order of quantities sold
    for (int i = 0; i < MAX_MENU - 1; i++) {
        for (int j = 0; j < MAX_MENU - i - 1; j++) {
            if (freqList[j].totalQty < freqList[j+1].totalQty) {
                ItemFreq temp = freqList[j];
                freqList[j] = freqList[j+1];
                freqList[j+1] = temp;
            }
        }
    }

    printf("\n2. Most-Ordered Items Ranking:\n");
    printf("%-5s %-25s %-10s\n", "Rank", "Item Name", "Qty Sold");
    printf("-----------------------------------------\n");
    for (int i = 0; i < 5; i++) {
        char name[30];
        for (int j = 0; j < MAX_MENU; j++) {
            if (menu[j].id == freqList[i].itemId) {
                strcpy(name, menu[j].name);
                break;
            }
        }
        printf("%-5d %-25s %-10d\n", i + 1, name, freqList[i].totalQty);
    }

    // 3. Busiest Hours Analysis
    int hourlyOrders[24] = {0};
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].status != CANCELLED) {
            hourlyOrders[orders[i].hour]++;
        }
    }

    int peakHour = 0;
    int maxOrdersInHour = 0;
    for (int h = 0; h < 24; h++) {
        if (hourlyOrders[h] > maxOrdersInHour) {
            maxOrdersInHour = hourlyOrders[h];
            peakHour = h;
        }
    }

    printf("\n3. Busiest Hour Analytics:\n");
    if (maxOrdersInHour > 0) {
        printf("Peak Hour: %02d:00 - %02d:00 with %d orders.\n", peakHour, (peakHour + 1) % 24, maxOrdersInHour);
    } else {
        printf("Not enough data to determine peak hours yet.\n");
    }
    printf("=========================================\n");
}