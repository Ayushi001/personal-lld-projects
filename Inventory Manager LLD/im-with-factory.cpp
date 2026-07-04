/******************************************************************************
An inventory management system is designed to efficiently track, manage, and optimize product inventory within a business.
The system needs to handle multiple products, track stock levels, process orders, and ensure timely replenishment.
The system should be able to accommodate different product types, handle various inventory operations, and provide accurate reporting.

Rules of the System:
Setup:
• The business has multiple warehouses and can store multiple product types.
• Products can be added, removed, or transferred between warehouses.
• Each product has attributes like SKU(Stock Keeping Unit - Unique Id of a Product), name, price, and quantity.
• The system tracks inventory levels and triggers alerts for low stock.


Operation:

• Users can add new products to the inventory.
• Stock levels are updated when products are received or shipped.
• The system generates reports on inventory status and movement.
• Inventory can be searched and filtered based on various criteria.

*******************************************************************************/

/*
Entities:
- Product : string sku, name, price, quantity/stock in the inventory

- Inventory : id, unordered_map<sku, Product>, vector<InventoryObserver> users -> observers for this inventory which will be notified
  - addProduct(int quantity)
  - removeProduct(string sku, int quantity)
  - notifyUsers()

- InventoryManager: unordered_map<id,Inventory> inventories , unordered_map<sku,vector<Inventory>> hasProductCache
  - addShipment(string inventoryId, string sku, int quantity)
  - removeShipment(string inventoryId, string sku, int quantity)
  // need locking for atomic transaction
  - transferShipment(string sourceInventoryId, string destinationInventoryId, string sku, int quantity)
  - vector<Inventory> hasProduct(string sku) // returns inventories which have product of productId=sku

- Interface InventoryObserver
  - UserObserver: userId, userName, emailId, sendMail()
  - DashboardObserver: triggerAlert()
Design Patterns:
- Observer pattern for sending alert on low stock of any product in the inventory
- Can use factory pattern for creating products of different product types, with ProductType enum
*/
#include <iostream>
#include <bits/stdc++.h>

using namespace std;
// Factory pattern for different ProductTypes
enum ProductType
{
    ELECTRONICS,
    GROCERY,
    CLOTHING
};
class Product
{
    string sku, name;
    int price, quantity;
    int threshold;

public:
    Product(string sku, string name, int price, int quantity) : sku(sku), name(name), price(price), quantity(quantity), threshold(100)
    {
    }
    int getQuantity()
    {
        return quantity;
    }
    void setQuantity(int q)
    {
        quantity = q;
    }
    int getThreshold()
    {
        return threshold;
    }
    string getSKU()
    {
        return sku;
    }
};
class ElectronicsProduct : public Product
{
    ProductType productType;
    int warrantyPeriod; // years
public:
    ElectronicsProduct(string sku, string name, int price, int quantity, int warrantyPeriod) : Product(
                                                                                                   sku,
                                                                                                   name,
                                                                                                   price,
                                                                                                   quantity),
                                                                                               productType(ProductType::ELECTRONICS), warrantyPeriod(warrantyPeriod)
    {
    }
};
class GroceryProduct : public Product
{
    int expiryTime; // days
    ProductType productType;

public:
    GroceryProduct(string sku, string name, int price, int quantity, int expiryTime) : Product(sku, name, price, quantity),
                                                                                       expiryTime(expiryTime),
                                                                                       productType(ProductType::GROCERY)
    {
    }
};
class ClothingProduct : public Product
{
    ProductType productType;
    int size;
    string color;

public:
    ClothingProduct(string sku, string name, int price, int quantity, int size, string color) : Product(sku, name, price, quantity),
                                                                                                size(size), color(color), productType(ProductType::CLOTHING)
    {
    }
};

class ProductFactory
{
public:
    Product *createProduct(string sku, string name, int price, int quantity, ProductType productType)
    {
        switch (productType)
        {
        case (ProductType::ELECTRONICS):
        {
            return new ElectronicsProduct(sku, name, price, quantity, 10);
        }
        case (ProductType::GROCERY):
        {
            return new GroceryProduct(sku, name, price, quantity, 2);
        }
        case (ProductType::CLOTHING):
        {
            return new ClothingProduct(sku, name, price, quantity, 5, "blue");
        }
        default:
            return new Product(sku, name, price, quantity);
        }
    }
};
// Observer interface
class InventoryObserver
{
public:
    virtual void notify(string msg) = 0;
};
class User : public InventoryObserver
{
    string userId, userName, email;

public:
    User(string userId, string userName, string email) : userId(userId), userName(userName), email(email)
    {
    }
    void notify(string msg) override
    {
        cout << "\nsending mail at: " << email << " with msg: " << msg;
    }
};

/*
- Inventory : id, unordered_map<sku, Product>, vector<InventoryObserver> users -> observers for this inventory which will be notified
  - addProduct(int quantity)
  - removeProduct(string sku, int quantity)
  - notifyUsers()
*/
class Inventory
{
    string id;
    unordered_map<string, Product *> productStock; //<sku,Product>
    vector<InventoryObserver *> UserObservers;

public:
    Inventory(string id, vector<InventoryObserver *> users) : id(id), UserObservers(users)
    {
    }
    void addProduct(Product *product, int quantity)
    {
        // Product* p=NULL;
        if (productStock.find(product->getSKU()) == productStock.end())
        {
            // create product using product factory here to cater to different product types
            // p=new Product(sku,"default product",100,quantity);
            productStock[product->getSKU()] = product;
        }

        // no need to get from map as its a pointer to same memory address
        // p=productStock[product->getSKU()];
        product->setQuantity(product->getQuantity() + quantity);
    }
    bool removeProduct(string sku, int quantity)
    {
        if (productStock.find(sku) == productStock.end())
        {
            cout << "ERROR: product not found in inventory";
            return false;
        }
        if (productStock[sku]->getQuantity() < quantity)
        {
            cout << "ERROR: not enough quantity";
            return false;
        }
        Product *product = productStock[sku];
        product->setQuantity(product->getQuantity() - quantity);
        if (product->getQuantity() < product->getThreshold())
        {
            for (auto user : UserObservers)
            {
                string notificationMsg = "productId: " + sku + " quantity " + to_string(product->getQuantity()) + " reached below threshold of " + to_string(product->getThreshold()) + "at inventoryId: " + id + "\n";
                user->notify(notificationMsg);
            }
        }
        return true; // success
    }
    Product *getProduct(string sku)
    {
        if (productStock.find(sku) == productStock.end())
            return NULL;
        return productStock[sku];
    }
    string getId()
    {
        return id;
    }
};

/*
- InventoryManager: unordered_map<id,Inventory> inventories , unordered_map<sku,vector<Inventory>> hasProductCache
  - addShipment(string inventoryId, string sku, int quantity)
  - removeShipment(string inventoryId, string sku, int quantity)
  // need locking for atomic transaction
  - transferShipment(string sourceInventoryId, string destinationInventoryId, string sku, int quantity)
  - vector<Inventory> hasProduct(string sku) // returns inventories which have product of productId=sku
*/
class InventoryManager
{
    // this is a shared resouce and if multiple threads of InventoryManager are running then introduce mutex
    // and introduce shared_lock<mutex> over all operations modifying inventories
    unordered_map<string, Inventory *> inventories;

public:
    InventoryManager(unordered_map<string, Inventory *> inventories) : inventories(inventories)
    {
    }
    void addShipment(string inventoryId, Product *product, int quantity)
    {
        if (inventories.find(inventoryId) == inventories.end())
        {
            cout << "ERROR: Invalid Inventory\n";
            return;
        }
        Inventory *inventory = inventories[inventoryId];
        inventory->addProduct(product, quantity);
    }
    void removeShipment(string inventoryId, string sku, int quantity)
    {
        if (inventories.find(inventoryId) == inventories.end())
        {
            cout << "ERROR: Invalid Inventory\n";
            return;
        }
        Inventory *inventory = inventories[inventoryId];
        inventory->removeProduct(sku, quantity);
    }
    void transferShipment(string sourceInventoryId, string destinationInventoryId, string sku, int quantity)
    {
        Product *product = inventories[sourceInventoryId]->getProduct(sku);
        bool removalSuccess = inventories[sourceInventoryId]->removeProduct(sku, quantity);
        if (removalSuccess)
            inventories[destinationInventoryId]->addProduct(product, quantity);
        else
            cout << "TRANSFER FAILED";
    }
    // returns list of all inventories which have this product
    vector<Inventory *> hasProduct(string sku)
    {
        vector<Inventory *> results;
        for (auto i : inventories)
        {
            if (i.second->getProduct(sku) != NULL)
            {
                results.push_back(i.second);
            }
        }
        return results;
    }
};
int main()
{
    unordered_map<string, Inventory *> testInventories;
    vector<InventoryObserver *> users;
    for (int i = 0; i < 3; i++)
    {
        users.push_back(new User("id" + to_string(i), to_string(i), to_string(i) + "@gmail.com"));
    }
    for (int i = 0; i < 2; i++)
    {
        Inventory *inventory = new Inventory(to_string(i), users);
        testInventories[inventory->getId()] = inventory;
    }
    InventoryManager *imClient = new InventoryManager(testInventories);
    ProductFactory *pf = new ProductFactory();
    Product *milk = pf->createProduct("0", "milk", 20, 5, ProductType::GROCERY);
    Product *watch = pf->createProduct("1", "watch", 2000, 10, ProductType::ELECTRONICS);
    Product *shirt = pf->createProduct("2", "shirt", 1500, 300, ProductType::CLOTHING);

    imClient->addShipment("0", milk, 0);
    imClient->addShipment("0", watch, 0);
    imClient->addShipment("0", shirt, 0);

    imClient->removeShipment("0", milk->getSKU(), 6);
    imClient->transferShipment("0", "1", watch->getSKU(), 6);
    // 	imClient->addShipment("0","p1",10);
    // 	imClient->removeShipment("0","p1",11);
    // 	imClient->removeShipment("0","p1",6);
    // 	imClient->transferShipment("0","1","p1",3);

    return 0;
}
/*
OUTPUT:
ERROR: not enough quantity
sending mail at: 0@gmail.com with msg: productId: 1 quantity 4 reached below threshold of 100at inventoryId: 0

sending mail at: 1@gmail.com with msg: productId: 1 quantity 4 reached below threshold of 100at inventoryId: 0

sending mail at: 2@gmail.com with msg: productId: 1 quantity 4 reached below threshold of 100at inventoryId: 0
*/