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
        cout << "sending mail at: " << email << " with msg: " << msg;
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
    void addProduct(string sku, int quantity)
    {
        Product *p = NULL;
        if (productStock.find(sku) == productStock.end())
        {
            p = new Product(sku, "default product", 100, quantity);
            productStock[sku] = p;
        }
        else
        {
            p = productStock[sku];
            p->setQuantity(p->getQuantity() + quantity);
        }
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
    void addShipment(string inventoryId, string sku, int quantity)
    {
        if (inventories.find(inventoryId) == inventories.end())
        {
            cout << "ERROR: Invalid Inventory\n";
            return;
        }
        Inventory *inventory = inventories[inventoryId];
        inventory->addProduct(sku, quantity);
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
        bool removalSuccess = inventories[sourceInventoryId]->removeProduct(sku, quantity);
        if (removalSuccess)
            inventories[destinationInventoryId]->addProduct(sku, quantity);
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
    imClient->addShipment("0", "p1", 10);
    imClient->removeShipment("0", "p1", 11);
    cout << "\n";
    imClient->removeShipment("0", "p1", 6);
    imClient->transferShipment("0", "1", "p1", 3);
    return 0;
}

/*
OUTPUT:
ERROR: not enough quantity
sending mail at: 0@gmail.com with msg: productId: p1 quantity 4 reached below threshold of 100at inventoryId: 0
sending mail at: 1@gmail.com with msg: productId: p1 quantity 4 reached below threshold of 100at inventoryId: 0
sending mail at: 2@gmail.com with msg: productId: p1 quantity 4 reached below threshold of 100at inventoryId: 0
sending mail at: 0@gmail.com with msg: productId: p1 quantity 1 reached below threshold of 100at inventoryId: 0
sending mail at: 1@gmail.com with msg: productId: p1 quantity 1 reached below threshold of 100at inventoryId: 0
sending mail at: 2@gmail.com with msg: productId: p1 quantity 1 reached below threshold of 100at inventoryId: 0
*/