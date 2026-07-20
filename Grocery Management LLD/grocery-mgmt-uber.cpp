/******************************************************************************
Grocery Management System - Alex Xu + uber LLD
Requirements:
- admins can add/remove items of diff types in a catalog
- inventory keeps track of stock levels and alert if stock goes too down
- different discounts can be applied on an item, on checkout max discount apply
- there should be flexible discount types like BUY 1 GET 1 , % discounts, and 1 discount
can be layered on top of another like a seasonal discount on top of flat % discount
- diff combos of discounts can also be applied like if item belongs to 1 category and meets a min price threshold
- during pricing calc, there can be surge pricing due to peak hrs or bad weather
- proper checkout with final price and inventory update
- should also offer item customisations like extra toppings,size upgrades?
*******************************************************************************/
/*
Entities
Item: id, category, name, price (can be made Interface)
 - int getPrice()
 - string getName()

// discounts can be based on category
enum Category{
    ELECTRONICS,
    CLOTHING,
    GROCERY
}
------------------------------------------------------------------------------------
// Customisations to users based on one specific grocery item using decorator pattern
CoffeeItem: public Item
 - id, category= Category::GROCERY, name, price
 - getters()

MilkItem: public Item
SugarItem: public Item

SugarDecorator: public Item
 - Item* item
 - int getPrice(){
     return item->getPrice()+ 10; // 10 is th extra sugar price
 }
 - string getName(){
     return item->getName() + "Sugar"
 }

MilkDecorator: public Item
- Item* item
- int getPrice(){
    return item->getName() + "Milk"
}
------------------------------------------------------------------------------------
ItemFactory:
- Item createItem(category ) factory pattern for item generation based on ItemType
------------------------------------------------------------------------------------
Catalog :  unordered_map<string, Item*> items // Catalog of items in the store <id, Item>
    - addItem() (admin level)
    - removeItem()

Inventory: unordered_map<string, int> stockLevel //<item id,stock>
    - addStock(id, int)
    - removeStock(id, int) // notify if stock below item threshold using observer pattern

OrderItem: Item item, int quantity , int orderTime
------------------------------------------------------------------------------------
Interface DiscountCriteria
  - bool isApplicable(OrderItem)

CategoryBasedCriteria: public DiscountCriteria
 - unordered_map<Category,bool> discountedCategory
 - bool isApplicable(OrderItem) // checks if item's category comes in the discountedCategory

PriceBasedCriteria: public DiscountCriteria
 - unordered_map<string, int> thresholdPrice //<item id, thresholdPrice>
 - bool isApplicable(OrderItem item) // applies if item's net price is above it's thresholdPrice

SeasonBasedCritaria: public DiscountCriteria
 - int saleMonth
 - bool isApplicable(OrderItem) // returns true if orderTime falls under saleMonth

CompositeCriteria: public DiscountCriteria
- vector<DiscountCriteria*> criterias
- bool isApplicable(OrderItem) // returns true if the orderItem satisfies ALL criterias
------------------------------------------------------------------------------------
Interface DiscountCalculationStrategy
  - int applyDiscount(OrderItem) // returns the discount value which is applied

PercentageDiscountStrategy: public DiscountCalculationStrategy
 - int percentage
 - int applyDiscount(OrderItem) // returns discount value % of total value

FixedPriceDiscountStrategy: public DiscountCalculationStrategy
 - int flatDiscount
 - int applyDiscount(OrderItem) // returns flatDisount which is constant discount applied
------------------------------------------------------------------------------------
For layered discounts (applying a percentage discount strategy on top of an already flat-discounted strategy)
PercentageDiscountDecorator: public DiscountCalculationStrategy
 - DiscountCalculationStrategy strategy
 - int percentage
 - int applyDiscount(OrderItem){
     return percentage/100 * strategy.applyDiscount(OrderItem)
 }

FixedPriceDiscountDecorator: public DiscountCalculationStrategy
 - DiscountCalculationStrategy strategy
 - int flatDiscount
 - int applyDiscount(OrderItem){
     return flatDiscount + strategy.applyDiscount(OrderItem)
 }
------------------------------------------------------------------------------------
*/

#include <bits/stdc++.h>
using namespace std;
/*
Item: id, category, name, price (can be made Interface)
 - int getPrice()
 - string getName()

// discounts can be based on category
enum Category{
    ELECTRONICS,
    CLOTHING,
    GROCERY
}
*/
enum Category
{
    GROCERY,
    ELECTRONICS
};
// for Decorator pattern we need pure Interface without member variables to wrap/decorate over
class Item
{
public:
    virtual string getName() = 0;
    virtual int getPrice() = 0;
    virtual int getId() = 0;
    virtual Category getCategory() = 0;
};
class BaseItem : public Item
{
protected:
    int id;
    Category category;
    string name;
    int price;

public:
    BaseItem(int id, Category category, string name, int price) : id(id), category(category), name(name), price(price)
    {
    }
    string getName() override
    {
        return name;
    }
    int getPrice() override
    {
        return price;
    }
    int getId() override
    {
        return id;
    }
    Category getCategory() override
    {
        return category;
    }
};
class CoffeeItem : public BaseItem
{
public:
    CoffeeItem() : BaseItem(1, Category::GROCERY, "Coffee", 100)
    {
    }
};
// Decorator pattern for item customisations for users at grocery store's app
// following are the top-ons (on any GROCERY Category type product)
class MilkDecorator : public Item
{
    Item *item;

public:
    // whenever a child class is created, it must call one of its parent class's constructors first.
    // since now the decorators are extending pure interfaces no need of any constructors
    MilkDecorator(Item *item) : item(item)
    {
    }
    string getName() override
    {
        return item->getName() + " Milk add-on";
    }
    int getPrice() override
    {
        return item->getPrice() + 20;
    }
    int getId() override
    {
        return item->getId();
    }
    Category getCategory() override
    {
        return item->getCategory();
    }
};
class SugarDecorator : public Item
{
    Item *item;

public:
    // base class must call its parent constructors first
    SugarDecorator(Item *item) : item(item)
    {
    }
    string getName() override
    {
        return item->getName() + " Extra Sugar";
    }
    int getPrice() override
    {
        return item->getPrice() + 2; // sugar add-on price
    }
    int getId() override
    {
        return item->getId();
    }
    Category getCategory() override
    {
        return item->getCategory();
    }
};
class VanillaDecorator : public Item
{
    Item *item;

public:
    // base class constructor must call their parent class first
    // since now the decorators are extending pure interfaces no need of any constructors
    VanillaDecorator(Item *item) : item(item)
    {
    }
    string getName()
    {
        return item->getName() + "Vanilla Add-on";
    }
    int getPrice()
    {
        return item->getPrice() + 25; // vanilla add-on price
    }
    int getId() override
    {
        return item->getId();
    }
    Category getCategory() override
    {
        return item->getCategory();
    }
};

class ElectronicsItem : public BaseItem
{
    int warrantyPeriod;

public:
    // base class must call their parent constructors first
    ElectronicsItem(string name, int price, int warrantyPeriod) : BaseItem(2, Category::ELECTRONICS, name, price),
                                                                  warrantyPeriod(warrantyPeriod)
    {
    }
};

/*
ItemFactory:
- Item createItem(category ) factory pattern for item generation based on ItemType
*/
class ItemFactory
{
public:
    Item *createItem(Category category, string itemName = "", int itemPrice = 0, int warrantyPeriod = 10)
    {
        switch (category)
        {
        case Category::GROCERY:
            return new CoffeeItem();
        case Category::ELECTRONICS:
        {
            return new ElectronicsItem(itemName, itemPrice, warrantyPeriod);
        }
        default:
            return NULL;
        }
    }
};

/*
Catalog :  unordered_map<int, Item*> items // Catalog of items in the store <id, Item>
    - addItem() (admin level)
    - removeItem()
*/
class Catalog
{
    unordered_map<int, Item *> items;

public:
    Catalog()
    {
    }
    void addItem(Item *item)
    {
        if (items.find(item->getId()) != items.end())
        {
            cout << "Item already in catalog";
            return;
        }
        items[item->getId()] = item;
    }
    Item *getItemDetails(int id)
    {
        if (items.find(id) == items.end())
        {
            cout << "Item doesn't exist in catalog";
            return NULL;
        }
        return items[id];
    }
};
// Observer pattern for stock levels
class StockObserver
{
public:
    virtual void notify(string msg) = 0;
};
class DashboardObserver : public StockObserver
{
    string dashboardId;

public:
    DashboardObserver(string id) : dashboardId(id)
    {
    }
    void notify(string msg) override
    {
        cout << "\nSending alert at dashboardId: " << dashboardId << " with log: " << msg;
    }
};
/*
Inventory: unordered_map<string, int> stockLevel //<item id,stock>
    - addStock(id, int)
    - removeStock(id, int) // notify if stock below item threshold using observer pattern
*/
class Inventory
{
    unordered_map<int, int> stock; //<item Id, quantity>
    vector<StockObserver *> observers;
    int threshold; // constant threshold across whole inventory, can be made item specific
public:
    Inventory(vector<StockObserver *> inventoryObservers, int globalThreshold) : observers(inventoryObservers), threshold(globalThreshold)
    {
    }
    void addStock(int id, int quantity)
    {
        stock[id] += quantity;
    }
    bool removeStock(int id, int quantity)
    {
        if (stock[id] < quantity)
        {
            cout << "Insufficient quantity\n";
            return false;
        }
        stock[id] -= quantity;
        if (stock[id] < threshold)
        {
            // notify all observers using observer pattern
            for (auto observer : observers)
            {
                string notificationBody = "Threshold breach for itemId: " + to_string(id) + " current stock level: " + to_string(stock[id]) + " Minimum Threashold needed: " + to_string(threshold);
                observer->notify(notificationBody);
            }
        }
        return true;
    }
    bool hasSufficientStock(int id, int quantity)
    {
        auto it = stock.find(id);
        if (it == stock.end() || it->second < quantity)
        {
            return false;
        }
        return true;
    }
};

/*
OrderItem: Item item, int quantity , int orderTime
*/
class OrderItem
{
    Item *item;
    int quantity;
    int orderMonth; // month at which the order got placed
public:
    OrderItem(Item *item, int quantity, int orderMonth) : item(item), quantity(quantity), orderMonth(orderMonth)
    {
    }
    Item *getItem()
    {
        return item;
    }
    int getQuantity()
    {
        return quantity;
    }
    int getOrderMonth()
    {
        return orderMonth;
    }
};
/*
Interface DiscountCriteria
  - bool isApplicable(OrderItem)

CategoryBasedCriteria: public DiscountCriteria
 - unordered_map<Category,bool> discountedCategory
 - bool isApplicable(OrderItem) // checks if item's category comes in the discountedCategory

PriceBasedCriteria: public DiscountCriteria
 - unordered_map<string, int> thresholdPrice //<item id, thresholdPrice>
 - bool isApplicable(OrderItem item) // applies if item's net price is above it's thresholdPrice

SeasonBasedCritaria: public DiscountCriteria
 - int saleMonth
 - bool isApplicable(OrderItem) // returns true if orderTime falls under saleMonth

CompositeCriteria: public DiscountCriteria
- vector<DiscountCriteria*> criterias
- bool isApplicable(OrderItem) // returns true if the orderItem satisfies ALL criterias
*/
class DiscountCriteria
{
public:
    virtual bool isApplicable(OrderItem *item) = 0; // returns true if the DiscountCriteria applies on OrderItem
};
// discount based on ordered item category
class CategoryBasedCriteria : public DiscountCriteria
{
    unordered_map<Category, bool> discountedCategories;

public:
    CategoryBasedCriteria()
    {
    }
    void addDiscountedCategory(Category category)
    {
        discountedCategories[category] = true;
    }
    void removeDiscountedCategory(Category category)
    {
        discountedCategories[category] = false;
    }
    bool isApplicable(OrderItem *orderItem) override
    {
        return discountedCategories[orderItem->getItem()->getCategory()];
    }
};
// discount based on ordered item's net price
class PriceBasedCriteria : public DiscountCriteria
{
    unordered_map<int, int> minPrice; // <item id, min price to be ordered to unlock discount on this item>
public:
    PriceBasedCriteria()
    {
    }
    void updateMinPrice(int itemId, int itemMinPrice)
    {
        minPrice[itemId] = itemMinPrice;
    }
    bool isApplicable(OrderItem *orderItem) override
    {
        int netOrderedPrice = orderItem->getItem()->getPrice() * orderItem->getQuantity();
        return netOrderedPrice >= minPrice[orderItem->getItem()->getId()] ? true : false;
    }
};
// Seasonal Criteria (eg if ordered in diwali month)
class SeasonBasedCritaria : public DiscountCriteria
{
    int saleMonth;

public:
    SeasonBasedCritaria()
    {
    }
    void setDiscountMonth(int month)
    {
        saleMonth = month;
    }
    bool isApplicable(OrderItem *orderItem) override
    {
        return orderItem->getOrderMonth() == saleMonth;
    }
};

// for multiple applicable discount criterias like to check if ordered for a discounted category in a discounted month
class CompositeDiscountCriteria : public DiscountCriteria
{
    vector<DiscountCriteria *> criterias;

public:
    CompositeDiscountCriteria(vector<DiscountCriteria *> criterias) : criterias(criterias)
    {
    }
    bool isApplicable(OrderItem *orderItem) override
    {
        bool ans = true;
        for (auto criteria : criterias)
        {
            ans &= criteria->isApplicable(orderItem);
        }
        return ans;
    }
};

/*
Interface DiscountCalculationStrategy
  - int applyDiscount(OrderItem) // returns the discount value which is applied

PercentageDiscountStrategy: public DiscountCalculationStrategy
 - int percentage
 - int applyDiscount(OrderItem) // returns discount value % of total value

FixedPriceDiscountStrategy: public DiscountCalculationStrategy
 - int flatDiscount
 - int applyDiscount(OrderItem) // returns flatDisount which is constant discount applied
*/
class DiscountCalculationStrategy
{
public:
    virtual int calculateDiscount(int originalPrice) = 0;
};
class PercentageDiscountStrategy : public DiscountCalculationStrategy
{
    int flatPercent;

public:
    PercentageDiscountStrategy(int percent) : flatPercent(percent)
    {
    }
    int calculateDiscount(int originalPrice) override
    {
        return originalPrice * flatPercent / 100;
    }
};
class ConstantDiscountStrategy : public DiscountCalculationStrategy
{
    int discountPrice;

public:
    ConstantDiscountStrategy(int discountPrice) : discountPrice(discountPrice)
    {
    }
    int calculateDiscount(int originalPrice) override
    {
        return discountPrice;
    }
};

// to enable layering of strategies on top of one another eg: applying percent based
// discount after constant discount
/*
For layered discounts (applying a percentage discount strategy on top of an already flat-discounted strategy)
PercentageDiscountDecorator: public DiscountCalculationStrategy
 - DiscountCalculationStrategy strategy
 - int percentage
 - int applyDiscount(OrderItem){
     return percentage/100 * strategy.applyDiscount(OrderItem)
 }

FixedPriceDiscountDecorator: public DiscountCalculationStrategy
 - DiscountCalculationStrategy strategy
 - int flatDiscount
 - int applyDiscount(OrderItem){
     return flatDiscount + strategy.applyDiscount(OrderItem)
 }
*/
class PercentageDiscountDecorator : public DiscountCalculationStrategy
{
    DiscountCalculationStrategy *strategy;
    int additionalPercent;

public:
    PercentageDiscountDecorator(DiscountCalculationStrategy *strategy, int percent) : strategy(strategy), additionalPercent(percent)
    {
    }
    int calculateDiscount(int originalPrice) override
    {
        int baseDiscount = strategy->calculateDiscount(originalPrice);
        int additionalDiscount = (originalPrice - baseDiscount) * additionalPercent / 100;

        return baseDiscount + additionalDiscount;
        // 		return (originalPrice - strategy->calculateDiscount(originalPrice))*additionalPercent/100;
    }
};
class ConstantDiscountDecorator : public DiscountCalculationStrategy
{
    int additionalDiscountPrice;
    DiscountCalculationStrategy *strategy;

public:
    ConstantDiscountDecorator(int discountPrice, DiscountCalculationStrategy *strategy) : additionalDiscountPrice(discountPrice), strategy(strategy)
    {
    }
    int calculateDiscount(int originalPrice) override
    {
        return strategy->calculateDiscount(originalPrice) + additionalDiscountPrice;
    }
};

class Order
{
    vector<OrderItem *> orderItems;
    int orderId;

public:
    Order(int id) : orderId(id)
    {
    }
    void addOrderItem(OrderItem *orderItem)
    {
        orderItems.push_back(orderItem);
    }
    vector<OrderItem *> getOrderItems()
    {
        return orderItems;
    }
    int getTotalPrice()
    {
        int price = 0;
        for (auto orderItem : orderItems)
        {
            price += orderItem->getItem()->getPrice() * orderItem->getQuantity();
        }
        return price;
    }
    int getOrderId()
    {
        return orderId;
    }
};
class DiscountPackage
{
    DiscountCriteria *criteria;
    DiscountCalculationStrategy *discountStrategy;

public:
    DiscountPackage(DiscountCriteria *criteria, DiscountCalculationStrategy *discountStrategy) : criteria(criteria), discountStrategy(discountStrategy)
    {
    }
    int applyDiscounts(Order *order)
    {
        int totalDiscount = 0;
        for (auto orderItem : order->getOrderItems())
        {
            if (criteria->isApplicable(orderItem))
            {
                int originalPrice = orderItem->getItem()->getPrice() * orderItem->getQuantity();
                totalDiscount += discountStrategy->calculateDiscount(originalPrice);
            }
        }
        return totalDiscount;
    }
};
class Receipt
{
    int receiptId;
    int totalDiscount;
    Order *order;
    int finalPrice;

public:
    Receipt(int id, int discount, Order *order, int price) : receiptId(id), totalDiscount(discount), order(order), finalPrice(price)
    {
    }
};
class CheckoutService
{
    Order *currentOrder;
    DiscountPackage *activeDiscount;
    Inventory *inventory;

public:
    CheckoutService(DiscountPackage *activeDiscount, Inventory *inventory) : activeDiscount(activeDiscount),
                                                                             inventory(inventory)
    {
    }
    void startNewOrder(int id)
    {
        currentOrder = new Order(id);
    }
    void addOrderItem(Item *item, int quantity)
    {
        OrderItem *orderItem = new OrderItem(item, quantity, 10);
        currentOrder->addOrderItem(orderItem);
    }
    Receipt *placeOrder()
    {
        int totalDiscount = activeDiscount->applyDiscounts(currentOrder);
        int finalPrice = currentOrder->getTotalPrice() - totalDiscount;
        for (auto orderItem : currentOrder->getOrderItems())
        {
            if (!inventory->hasSufficientStock(orderItem->getItem()->getId(), orderItem->getQuantity()))
                continue;
            bool isSuccess = inventory->removeStock(orderItem->getItem()->getId(), orderItem->getQuantity());
            if (!isSuccess)
            {
                cout << "Placing order failed";
                return NULL;
            }
        }
        Receipt *receipt = new Receipt(currentOrder->getOrderId(), totalDiscount, currentOrder, finalPrice);
        return receipt;
    }
};
// Facade class
class StoreManager
{
    Inventory *inventory;
    Catalog *catalog;
    DiscountPackage *activeDiscount;
    CheckoutService *checkoutService;

public:
    StoreManager(Inventory *inventory, Catalog *catalog, DiscountPackage *activeDiscount) : inventory(inventory),
                                                                                            catalog(catalog),
                                                                                            activeDiscount(activeDiscount)
    {
        this->checkoutService = new CheckoutService(activeDiscount, inventory);
    }
    void addOrderItem(Item *item, int quantity)
    {
        if (!inventory->hasSufficientStock(item->getId(), quantity))
        {
            cout << "Insufficient";
            return;
        }
        checkoutService->addOrderItem(item, quantity);
    }
    Receipt *placeOrder()
    {
        Receipt *receipt = checkoutService->placeOrder();
        return receipt;
    }
    void addStock(int itemId, int quantity)
    {
        inventory->addStock(itemId, quantity);
    }
};

int main()
{
    std::cout << "Hello World";

    return 0;
}