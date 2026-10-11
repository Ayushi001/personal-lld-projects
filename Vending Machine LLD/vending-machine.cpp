/******************************************************************************
Vending Machine LLD - ALex Xu chp 7

Requirements:
- user selects product and inserts coins and VM dispenses
- Each rack has product of same type
- Admins can add, remove product from VM
- VM system maintains the inventory and alerts admins if stock of a product is too low
*******************************************************************************/
#include <iostream>
#include<bits/stdc++.h>
/*
LLD patterns:
- State design for transitioning across various VM states
- Strategy patten for payment processing via cash, card, upi
- Observer pattern for stock alerts if curr stock below threshold and alerts the admin
- Facade pattern for user facing VM system class
*/

using namespace std;
//Core Entities
class Product {
	int id;
	string name;
	int price;

public:
	Product(int id,string name,int price):id(id),name(name), price(price) {
	}
	string getName() {
		return name;
	}
	int getPrice() {
		return price;
	}
	int getId()
	{
		return id;
	}
};
class Rack {
	int id;
};

class InventoryObserver {
public:
	virtual void notify(string msg)=0;
}; //to alert admins
class AdminObserver: public InventoryObserver {
	string adminMail;
public:
	AdminObserver(string adminMail):adminMail(adminMail) {
	}
	void notify(string msg) {
		// Email API to send mail
		cout<<msg;
		cout<<"Mail sent at: "<<adminMail;
	}
};
//maintains stock levels and alerts if stocks fall below threshold
class Inventory {
	unordered_map<int,int> productStock; //<productId, quantity>
	int threshold=50;
	vector<InventoryObserver*> observers;
public:
	Inventory(unordered_map<int,int> initialStock, vector<InventoryObserver*> observers):productStock(initialStock),
		observers(observers) {
	}
	void setThreshold(int minThreshold) {
		threshold=minThreshold;
	}
	void addStock(int productId,int quantity) {
		productStock[productId]+=quantity;
	}
	bool removeStock(int productId, int quantity)
	{
		if(productStock[productId]<quantity) {
			cout<<"[ERROR] Insufficient stock";
			return false;
		}
		productStock[productId]-=quantity;
		if(productStock[productId]<threshold)
		{
			// alert the admins/inventory observers
			for(auto observer:observers)
			{
				string notifyMsg="[STOCK_ALERT] stock for productId: "+to_string(productId)
				                 +" is"
				                 +to_string(productStock[productId])+"below threshold of: "+to_string(threshold);
				observer->notify(notifyMsg);
			}
		}
		return true;
	}
};



// Not needed as PaymentProcessingState is the PaymentProcessor only
// class PaymentProcessor;
class PaymentStrategy {
	int userBalance;
public:
	PaymentStrategy() {}
	virtual void setUserBalance(int userDetail)=0;
	bool processPayment(int amount) {
		if(userBalance<amount)
		{
			cout<<"[ERROR] Payment processing failed: Insufficient User balance, min balance needed: "<<amount;
			return false;
		}
		userBalance-=amount;
		cout<<"[INFO] Transaction Successful";
		return true;
	}
};
class UPIPaymentStrategy: public PaymentStrategy {
	int userBalance;
public:
	void setUserBalance(int upiId) override {
		// api to fetch user balance from input upiId, currently hardcoding
		userBalance=1000;
	}
};
class CardPaymentStrategy: public PaymentStrategy {
	int userBalance;
public:
	void setUserBalance(int cardNumber) override {
		// api to fetch user balance from input card cardNumber, currently hardcoding
		userBalance=1000;
	}
};
class CashPaymentStrategy: public PaymentStrategy {
	int userBalance;
public:
	void setUserBalance(int cashInput) override {
		// updates the balance from input cash user inserted in vending machine
		userBalance=cashInput;
	}
};

//to maintain VM's curr state
class VMState; // forward declarations
// context having current state status of vending machine
class VMStateContext {
	VMState* currState;
	PaymentStrategy* selectedPaymentMode;
	Product* selectedProduct;
	int selectedQty;
	Inventory* inventory;
	// mostly constant
	unordered_map<int,Product*> catalog; //<productId, Product*>
public:
	VMStateContext(Inventory* inventory, VMState* initialState, unordered_map<int,Product*> catalog):inventory(inventory)
		, catalog(catalog) {
		// currState=new IdleState(); //circular dependency
		currState=initialState;
	}
	void setState(VMState* state)
	{
		currState=state;
	}
	void setPaymentMode(PaymentStrategy* paymentMode)
	{
		selectedPaymentMode=paymentMode;
	}
	void setSelectedProduct(Product* product)
	{
		selectedProduct=product;
	}
	void setSelectedQty(int quantity)
	{
		selectedQty=quantity;
	}
	Product* getSelectedProduct() {
		return selectedProduct;
	}
	PaymentStrategy* getPaymentMode() {
		return selectedPaymentMode;
	}
	void updateInventory() {
		// assuming inventory will always have selected quantity
		bool isSuccess=inventory->removeStock(selectedProduct->getId(),selectedQty);
		if(!isSuccess)
		{
			// here then the payment needs to be refunded
		}
	}
	VMState* getState() {
		return currState;
	}
	Product* getProductDetails(int productId)
	{
		return catalog[productId];
	}
};

// vending Machine state interface
class VMState {
public:
	// pure virtual functions
	// virtual void selectPaymentMode(VMStateContext* ctx)=0;
	// virtual void selectProduct(VMStateContext* ctx)=0;
	// virtual void insertMoney(VMStateContext* ctx,int amount)=0;
	// virtual void executeTransaction(VMStateContext* ctx)=0; //validate balance from payment mode and execute
	// virtual void dispenseProduct(VMStateContext* ctx)=0;

	// regular virtual functions with default behavior
	virtual void selectPaymentMode(VMStateContext* ctx, PaymentStrategy* paymentMode, bool isCash) {
		cout<<"[ERROR] payment mode already selected";
	}
	virtual void insertCash(VMStateContext *ctx) {
		cout<<"[INFO] Cash Insertion not needed, select product now";
	}
	// take the input as rack id and fetch the product from there
	virtual void selectProduct(VMStateContext* ctx, Product* selectedProduct, int selectedQty) {
		cout<<"[ERROR] select payment mode first"; // use singleton pattern here later for logger instance
	}
	virtual void executeTransaction(VMStateContext* ctx) {
		cout<<"[ERROR] select product first";
	}
	virtual void dispenseProduct(VMStateContext* ctx) {
		cout<<"[ERROR] let payment process succeed";
	}
};

class IdleState: public VMState {
public:
	// method forward declarations due to circular dependencies btwn states
	// another way is to define all VMStates in reverse order
	void selectPaymentMode(VMStateContext* ctx, PaymentStrategy* paymentMode, bool isCash) override;
};
class InsertCashState: public VMState {
public:
	void insertCash(VMStateContext* ctx) override;
};
class ProductSelectionState: public VMState {
public:
	void selectProduct(VMStateContext* ctx, Product* selectedProduct, int selectedQty) override ;
};
class PaymentProcessingState: public VMState {
public:
	void executeTransaction(VMStateContext *ctx) override;
};
class DispenseState: public VMState {
public:
	void dispenseProduct(VMStateContext* ctx) override;
};

// State Method Implementation to avoid cyclic dependencies btwn states
void IdleState::selectPaymentMode(VMStateContext* ctx, PaymentStrategy* paymentMode, bool isCash) {
	ctx->setPaymentMode(paymentMode);
	if(isCash)
	{
		ctx->setState(new InsertCashState());
	}
	else
		ctx->setState(new ProductSelectionState());
}
void InsertCashState::insertCash(VMStateContext* ctx) {
	int cashInput;
	cout<<"Insert Cash: ";
	cin>>cashInput;
	ctx->getPaymentMode()->setUserBalance(cashInput);
	ctx->setState(new ProductSelectionState());
}
void ProductSelectionState::selectProduct(VMStateContext* ctx, Product* selectedProduct, int selectedQty) {
	ctx->setSelectedProduct(selectedProduct);
	ctx->setState(new PaymentProcessingState());
}
void PaymentProcessingState::executeTransaction(VMStateContext *ctx) {
	bool isSuccess=ctx->getPaymentMode()->processPayment(ctx->getSelectedProduct()->getPrice());
	if(!isSuccess)
	{
		cout<<"\nTransaction Failed";
		ctx->setState(new ProductSelectionState());
	}
	else {
		cout<<"\nTransaction Succeeded";
		ctx->setState(new DispenseState());
	}
}
void DispenseState::dispenseProduct(VMStateContext* ctx) {
	cout<<"Dispensing product: "<<ctx->getSelectedProduct()->getName();
	// update the inventory
	ctx->updateInventory();
	ctx->setState(new IdleState());
}

// StateContext class is the middleware btwn client and actual individual state execution classes
// this client is directly interacting with users
class VMClient {
	VMStateContext *ctx;
	// do this hard-coded initialisation from int main() like a unit-tested setup
	// and keep the class func generic
	// void initiateSetup(){
	//     Inventory* inventory=new
	//     ctx=new VMStateContext(inventory, new IdleState());
	// }
public:
	VMClient(VMStateContext* ctx):ctx(ctx) {
		// initiateSetup();
		ctx->setState(new IdleState());
	}
	void selectPaymentStep() {
		PaymentStrategy* strategy;
		cout<<"1. CASH, 2.UPI, 3.CARD\n";
		int mode;
		cin>>mode;
		switch(mode)
		{
		case 1:
			strategy=new  CashPaymentStrategy();
		case 2:
			strategy= new UPIPaymentStrategy();
		case 3:
			strategy= new CardPaymentStrategy();
		default:
			cout<<"Invalid input";
		}
		ctx->getState()->selectPaymentMode(ctx,strategy,mode==1);
	}
	void insertCashStep() {
		ctx->getState()->insertCash(ctx);
	}
	void productSelectionStep() {
		cout<<"Enter Product ID: ";
		int productId;
		cin>>productId;
		Product* product=ctx->getProductDetails(productId);
		cout<<"\n Enter Quantity: ";
		int quantity;
		cin>>quantity;
		ctx->getState()->selectProduct(ctx,product,quantity);
	}
	void paymentProcessingStep() {
		ctx->getState()->executeTransaction(ctx);
	}
	void dispensingProductStep() {
		ctx->getState()->dispenseProduct(ctx);
	}
	void startVending()
	{
		// ideally each of the state's method implementation should throw error
		// so none of the remaning steps complete
		selectPaymentStep();
		insertCashStep();
		productSelectionStep();
		paymentProcessingStep();
		dispensingProductStep();
		cout<<"\n[INFO] Vending Complete";
	}

}; // Facade Class

int main()
{
	cout<<"Hello World";
	// Build Vending Machine Context for VMClient
	unordered_map<int,Product*>catalog= {
		{1, new Product(1,"Milk",20) },
		{2, new Product(2, "Chocolate", 10) },
		{3, new Product(3,"Popcorn",25)}
	};
	unordered_map<int,int> initialStock= {
		{1,50},
		{2,100},
		{3,90}
	};
	vector<InventoryObserver*>observers= {
		new AdminObserver("ayushiverma@oncall.com"),
		new AdminObserver("admin-manager@oncall.com")
	};
	Inventory* inventory=new Inventory(initialStock,observers);
	VMStateContext* ctx= new VMStateContext(inventory,new IdleState(),catalog);

	VMClient*vmClient=new VMClient(ctx);
	vmClient->startVending();
	return 0;
}
/*
OUTPUT:
Hello World1. CASH, 2.UPI, 3.CARD
2
Invalid input[INFO] Cash Insertion not needed, select product nowEnter Product ID: 1

 Enter Quantity: 5
[ERROR] Payment processing failed: Insufficient User balance, min balance needed: 20
Transaction Failed[ERROR] let payment process succeed
[INFO] Vending Complete
*/