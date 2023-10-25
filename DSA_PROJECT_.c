#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INVESTMENTS 1000

int recommendationStartIndex = 0;

// Define Investment structure
typedef struct {
    int index;  // Add an index field to keep track of position
    char name[100];
    char type[50];
    double currentPrice;
    char riskLevel[10];
    double expectedReturn;
    double correlationWithMarket;
} Investment;

// Define Graph structure
typedef struct GraphNode {
    Investment* investment;
    double correlation;  // Represents the correlation value between two investments
    struct GraphNode* next;
} GraphNode;

typedef struct Graph {
    GraphNode* vertices[MAX_INVESTMENTS];
} Graph;

GraphNode* createNode(Investment* investment, double correlation) {
    GraphNode* newNode = (GraphNode*)malloc(sizeof(GraphNode));
    newNode->investment = investment;
    newNode->correlation = correlation;
    newNode->next = NULL;
    return newNode;
}

void addEdge(Graph* graph, Investment* srcInvestment, Investment* destInvestment, double correlation) {
    GraphNode* newNode = createNode(destInvestment, correlation);
    newNode->next = graph->vertices[srcInvestment->index];
    graph->vertices[srcInvestment->index] = newNode;
}

Graph* createGraph() {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    for(int i = 0; i < MAX_INVESTMENTS; i++) {
        graph->vertices[i] = NULL;
    }
    return graph;
}

int riskLevelChosen[3] = {0};  // 0: Low, 1: Medium, 2: High

// Static counters and last recommendation indices
static int highRiskCounter = 0;
static int mediumRiskCounter = 0;
static int lowRiskCounter = 0;

static int lastRecommendedIndexHigh = 0;
static int lastRecommendedIndexMedium = 0;
static int lastRecommendedIndexLow = 0;

// Define AVL Node structure
typedef struct AVLNode {
    Investment data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

int height(AVLNode* N) {
    if (N == NULL)
        return 0;
    return N->height;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

AVLNode* newNode(Investment inv) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->data = inv;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

AVLNode* rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

AVLNode* leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

int getBalance(AVLNode* N) {
    if (N == NULL)
        return 0;
    return height(N->left) - height(N->right);
}

AVLNode* insertAVL(AVLNode* node, Investment inv) {
    if (node == NULL)
        return newNode(inv);

    if (inv.currentPrice < node->data.currentPrice)
        node->left = insertAVL(node->left, inv);
    else if (inv.currentPrice > node->data.currentPrice)
        node->right = insertAVL(node->right, inv);
    else
        return node;

    node->height = 1 + max(height(node->left), height(node->right));

    int balance = getBalance(node);

    if (balance > 1 && inv.currentPrice < node->left->data.currentPrice)
        return rightRotate(node);

    if (balance < -1 && inv.currentPrice > node->right->data.currentPrice)
        return leftRotate(node);

    if (balance > 1 && inv.currentPrice > node->left->data.currentPrice) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    if (balance < -1 && inv.currentPrice < node->right->data.currentPrice) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

typedef struct {
    Investment investment;
    double amountInvested;
} PortfolioItem;

Investment investments[MAX_INVESTMENTS];
int investmentCount = 0;
PortfolioItem portfolio[MAX_INVESTMENTS];
int portfolioCount = 0;
AVLNode* avlRoot = NULL;

void readCSVAndBuildGraph(const char* filename, Graph* graph) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    char line[256];
    fgets(line, sizeof(line), file);  // Skip header

    while (fgets(line, sizeof(line), file)) {
        Investment inv;
        sscanf(line, "%[^,],%[^,],%lf,%[^,],%lf,%lf", 
               inv.name, inv.type, &inv.currentPrice, inv.riskLevel, &inv.expectedReturn, &inv.correlationWithMarket);
        
        inv.index = investmentCount;
        investments[investmentCount++] = inv;
        avlRoot = insertAVL(avlRoot, inv);
    }

    for (int i = 0; i < investmentCount; i++) {
        for (int j = 0; j < investmentCount; j++) {
            if (i != j) {
                addEdge(graph, &investments[i], &investments[j], investments[j].correlationWithMarket);
            }
        }
    }

    fclose(file);
}

void recommendBasedOnCorrelation(Graph* graph, const char* investmentName) {
    printf("\nRecommendations based on correlation with %s:\n", investmentName);
    
    for (int i = 0; i < investmentCount; i++) {
        if (strcmp(investments[i].name, investmentName) == 0) {
            GraphNode* node = graph->vertices[i];
            while (node) {
                printf("Investment: %s, Correlation: %.2lf%%\n", node->investment->name, node->correlation * 100);
                node = node->next;
            }
            break;
        }
    }
}

// Static counters for rotation
static int recommendationRotation = 0;

void sortInvestmentsByReturn() {
    for (int i = 0; i < investmentCount - 1; i++) {
        for (int j = 0; j < investmentCount - i - 1; j++) {
            if (investments[j].expectedReturn < investments[j + 1].expectedReturn || 
               (investments[j].expectedReturn == investments[j + 1].expectedReturn && rand() % 2)) {
                Investment temp = investments[j];
                investments[j] = investments[j + 1];
                investments[j + 1] = temp;
            }
        }
    }

    recommendationRotation = (recommendationRotation + 1) % investmentCount;  // Rotate recommendation starting point
}



void rotateInvestments() {
    Investment tempInvestments[MAX_INVESTMENTS];
    for (int i = 0; i < investmentCount; i++) {
        tempInvestments[i] = investments[(i + recommendationStartIndex) % investmentCount];
    }
    for (int i = 0; i < investmentCount; i++) {
        investments[i] = tempInvestments[i];
    }

    recommendationStartIndex = (recommendationStartIndex + 1) % investmentCount;
}


double getRiskPercentage(const char* riskLevel) {
    if (strcmp(riskLevel, "Low") == 0) {
        return 10.0;
    } else if (strcmp(riskLevel, "Medium") == 0) {
        return 25.0;
    } else if (strcmp(riskLevel, "High") == 0) {
        return 50.0;
    } else {
        return 0.0;
    }
}

void adjustReturnsBasedOnUserRiskPreference(const char* riskPreference) {
    double reductionFactor = 1.0;

    if (strcmp(riskPreference, "Low") == 0 && riskLevelChosen[0]) {
        if (++lowRiskCounter % 9 == 0) {
            reductionFactor = 0.9;
        }
    } else if (strcmp(riskPreference, "Medium") == 0 && riskLevelChosen[1]) {
        if (++mediumRiskCounter % 4 == 0) {
            reductionFactor = 0.75;
        }
    } else if (strcmp(riskPreference, "High") == 0 && riskLevelChosen[2]) {
        if (++highRiskCounter % 2 == 0) {
            reductionFactor = 0.5;
        }
    }

    for (int i = 0; i < investmentCount; i++) {
        investments[i].expectedReturn *= reductionFactor;
    }

    if (strcmp(riskPreference, "Low") == 0) riskLevelChosen[0] = 1;
    else if (strcmp(riskPreference, "Medium") == 0) riskLevelChosen[1] = 1;
    else if (strcmp(riskPreference, "High") == 0) riskLevelChosen[2] = 1;
}

int main() {
    Graph* graph = createGraph();
    memset(riskLevelChosen, 0, sizeof(riskLevelChosen));
    investmentCount = 0;
    avlRoot = NULL;
    readCSVAndBuildGraph("C:\\Users\\Parth Tripathi\\Downloads\\investment_data.csv", graph);
    memset(portfolio, 0, sizeof(portfolio));
    portfolioCount = 0;

    while (1) {
        memset(portfolio, 0, sizeof(portfolio));
        portfolioCount = 0;

        char userName[100];
        double budget;
        char riskPreference[10];

        printf("Welcome to the Portfolio Management System!\n");
        printf("Enter your name: ");
        scanf(" %[^\n]", userName);
        printf("Hello, %s!\n", userName);
    
        printf("Enter your budget (in INR): ");
        scanf("%lf", &budget);
        if(budget <= 0){
            printf("Please enter a positive investment amount.\n");
            continue;
        }
    
        printf("Enter your risk preference (High, Medium, Low): ");
scanf("%s", riskPreference);

// Convert the input to title case for comparison
for (int i = 0; riskPreference[i]; i++) {
    riskPreference[i] = (i == 0) ? toupper(riskPreference[i]) : tolower(riskPreference[i]);
}

while (strcmp(riskPreference, "High") != 0 && strcmp(riskPreference, "Medium") != 0 && strcmp(riskPreference, "Low") != 0) {
    printf("Invalid choice. Please enter 'High', 'Medium', or 'Low': ");
    scanf("%s", riskPreference);

    // Convert the input to title case for comparison
    for (int i = 0; riskPreference[i]; i++) {
        riskPreference[i] = (i == 0) ? toupper(riskPreference[i]) : tolower(riskPreference[i]);
    }
	}

		double riskPercentage = getRiskPercentage(riskPreference);
		printf("\nBased on your %s risk preference, there's a %.2lf%% annual chance your investments could return a lower return.\n", riskPreference, riskPercentage);		


        printf("\nWould you like us to recommend a portfolio or choose your own diversification? (Enter 'recommend' or 'choose'): ");
        char choice[10];
        scanf("%s", choice);

        while (strcmp(choice, "recommend") != 0 && strcmp(choice, "choose") != 0) {
            printf("Please enter a valid input (recommend or choose): ");
            scanf("%s", choice);
        }

        adjustReturnsBasedOnUserRiskPreference(riskPreference);

        if (strcmp(choice, "recommend") == 0) {
            sortInvestmentsByReturn();
            printf("\nRecommended Investments for %s risk:\n", riskPreference);
    
            char typesChosen[MAX_INVESTMENTS][50] = {0};
            int typesCount = 0;
    
            for (int i = 0; i < investmentCount && budget > 0; i++) {
                int idx = rand() % investmentCount; // Random Investment Selection
                int typeAlreadyChosen = 0;
                for (int j = 0; j < typesCount; j++) {
                    if (strcmp(investments[idx].type, typesChosen[j]) == 0) {
                        typeAlreadyChosen = 1;
                        break;
                    }
                }

                if (!typeAlreadyChosen && strcmp(investments[idx].riskLevel, riskPreference) == 0) {
                    strcpy(typesChosen[typesCount++], investments[idx].type);
                    double fractionOfBudget = budget * ((rand() % 11 + 15) / 100.0);  // Dynamic Weight Allocation: 15% to 25%
                    double amountToInvest = fractionOfBudget < investments[idx].currentPrice ? fractionOfBudget : investments[idx].currentPrice;
                    budget -= amountToInvest;

                    portfolio[portfolioCount].investment = investments[idx];
                    portfolio[portfolioCount].amountInvested = amountToInvest;
                    portfolioCount++;
                }
            }
    
            printf("\nYour recommended portfolio is:\n");
            double totalExpectedReturn = 0.0;
            for (int i = 0; i < portfolioCount; i++) {
                printf("Investment: %s, Type: %s, Amount: INR %.2lf\n", portfolio[i].investment.name, portfolio[i].investment.type, portfolio[i].amountInvested);
                totalExpectedReturn += portfolio[i].investment.expectedReturn / 100 * portfolio[i].amountInvested;
            }
            printf("\nExpected annual return: INR %.2lf, which is %.2lf%% of the invested amount.\n", totalExpectedReturn, (totalExpectedReturn / (5000 - budget)) * 100);
        } 
	    else if (strcmp(choice, "choose") == 0) {
    printf("\nAvailable investments for %s risk:\n", riskPreference);
    while (budget > 0) {
        for (int i = 0; i < investmentCount; i++) {
            if (strcmp(investments[i].riskLevel, riskPreference) == 0) {
                printf("%d. %s (%s) - Current Price: INR %.2lf, Expected Return: %.2lf%%\n", 
                    i+1, investments[i].name, investments[i].type, investments[i].currentPrice, investments[i].expectedReturn);
            }
        }

        int chosenIndex;
        printf("\nChoose an investment by entering its index (or -1 to stop): ");
        scanf("%d", &chosenIndex);

        if (chosenIndex == -1) {
            break;
        }

        if (chosenIndex < 1 || chosenIndex > investmentCount) {
            printf("Invalid index. Please choose again.\n");
            continue;
        }

        double amountToInvest;
        printf("Enter the amount you want to invest in %s: INR ", investments[chosenIndex - 1].name);
        scanf("%lf", &amountToInvest);

        if (amountToInvest > budget) {
            printf("You don't have enough budget. You have INR %.2lf left.\n", budget);
            continue;
        }

        budget -= amountToInvest;

        portfolio[portfolioCount].investment = investments[chosenIndex - 1];
        portfolio[portfolioCount].amountInvested = amountToInvest;
        portfolioCount++;

        printf("Invested INR%.2lf in %s. Remaining budget: INR %.2lf.\n", amountToInvest, investments[chosenIndex - 1].name, budget);
    }

    printf("\nYour chosen portfolio is:\n");
    double totalExpectedReturn = 0.0;
    for (int i = 0; i < portfolioCount; i++) {
        printf("Investment: %s, Type: %s, Amount: INR %.2lf\n", portfolio[i].investment.name, portfolio[i].investment.type, portfolio[i].amountInvested);
        totalExpectedReturn += portfolio[i].investment.expectedReturn / 100 * portfolio[i].amountInvested;
    }
    printf("\nExpected annual return: INR %.2lf, which is %.2lf%% of the invested amount.\n", totalExpectedReturn, (totalExpectedReturn / (5000 - budget)) * 100);
}


        printf("\nWould you like to enter details for the next user? (Enter 'yes' or 'no'): ");
        char nextUserChoice[5];
        scanf("%s", nextUserChoice);
        if (strcmp(nextUserChoice, "no") == 0) {
            printf("Thank you for using the Portfolio Management System! Goodbye!\n");
            break;
        }
    }
    return 0;
}


