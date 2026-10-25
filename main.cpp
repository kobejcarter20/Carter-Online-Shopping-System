#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// ============================================================
// Product Class
// ============================================================

class Product {
private:
    int id;
    std::string name;
    std::string description;
    double price;
    int stockQuantity;
    mutable std::mutex stockMutex;

public:
    Product(int productId,
            std::string productName,
            std::string productDescription,
            double productPrice,
            int productStock)
        : id(productId),
          name(std::move(productName)),
          description(std::move(productDescription)),
          price(productPrice),
          stockQuantity(productStock) {

        if (id < 1 || name.empty() || price < 0 || stockQuantity < 0) {
            throw std::invalid_argument("Invalid product information.");
        }
    }

    // Prevent copying because the Product contains a mutex.
    Product(const Product&) = delete;
    Product& operator=(const Product&) = delete;

    int getId() const {
        return id;
    }

    std::string getName() const {
        return name;
    }

    std::string getDescription() const {
        return description;
    }

    double getPrice() const {
        return price;
    }

    int getStockQuantity() const {
        std::lock_guard<std::mutex> lock(stockMutex);
        return stockQuantity;
    }

    void updateProduct(const std::string& newName,
                       const std::string& newDescription,
                       double newPrice) {
        if (newName.empty() || newPrice < 0) {
            throw std::invalid_argument("Invalid product update.");
        }

        name = newName;
        description = newDescription;
        price = newPrice;
    }

    bool reserveStock(int quantity) {
        if (quantity <= 0) {
            return false;
        }

        std::lock_guard<std::mutex> lock(stockMutex);

        if (quantity > stockQuantity) {
            return false;
        }

        stockQuantity -= quantity;
        return true;
    }

    void restoreStock(int quantity) {
        if (quantity <= 0) {
            return;
        }

        std::lock_guard<std::mutex> lock(stockMutex);
        stockQuantity += quantity;
    }
};

// ============================================================
// Catalog Class
// ============================================================

class Catalog {
private:
    std::vector<std::shared_ptr<Product>> products;
    mutable std::mutex catalogMutex;

public:
    bool addProduct(const std::shared_ptr<Product>& product) {
        if (!product) {
            return false;
        }

        std::lock_guard<std::mutex> lock(catalogMutex);

        for (const auto& existingProduct : products) {
            if (existingProduct->getId() == product->getId()) {
                return false;
            }
        }

        products.push_back(product);
        return true;
    }

    bool removeProduct(int productId) {
        std::lock_guard<std::mutex> lock(catalogMutex);

        auto productIterator = std::remove_if(
            products.begin(),
            products.end(),
            [productId](const std::shared_ptr<Product>& product) {
                return product->getId() == productId;
            }
        );

        if (productIterator == products.end()) {
            return false;
        }

        products.erase(productIterator, products.end());
        return true;
    }

    std::shared_ptr<Product> findProduct(int productId) const {
        std::lock_guard<std::mutex> lock(catalogMutex);

        for (const auto& product : products) {
            if (product->getId() == productId) {
                return product;
            }
        }

        return nullptr;
    }

    void displayProducts() const {
        std::lock_guard<std::mutex> lock(catalogMutex);

        std::cout << "\n============== PRODUCT CATALOG ==============\n";
        std::cout << std::left
                  << std::setw(5) << "ID"
                  << std::setw(22) << "Product"
                  << std::setw(14) << "Price"
                  << "Stock\n";

        std::cout << "---------------------------------------------\n";

        for (const auto& product : products) {
            std::cout << std::left
                      << std::setw(5) << product->getId()
                      << std::setw(22) << product->getName()
                      << "$" << std::setw(13)
                      << std::fixed << std::setprecision(2)
                      << product->getPrice()
                      << product->getStockQuantity()
                      << '\n';
        }
    }
};

// ============================================================
// Shopping Cart Class
// ============================================================

struct CartItem {
    std::shared_ptr<Product> product;
    int quantity;
};

class ShoppingCart {
private:
    std::vector<CartItem> items;
    mutable std::mutex cartMutex;

public:
    bool addProduct(const std::shared_ptr<Product>& product, int quantity) {
        if (!product || quantity <= 0) {
            return false;
        }

        std::lock_guard<std::mutex> lock(cartMutex);

        for (auto& item : items) {
            if (item.product->getId() == product->getId()) {
                item.quantity += quantity;
                return true;
            }
        }

        items.push_back({product, quantity});
        return true;
    }

    bool removeProduct(int productId) {
        std::lock_guard<std::mutex> lock(cartMutex);

        auto itemIterator = std::remove_if(
            items.begin(),
            items.end(),
            [productId](const CartItem& item) {
                return item.product->getId() == productId;
            }
        );

        if (itemIterator == items.end()) {
            return false;
        }

        items.erase(itemIterator, items.end());
        return true;
    }

    bool updateQuantity(int productId, int newQuantity) {
        if (newQuantity <= 0) {
            return removeProduct(productId);
        }

        std::lock_guard<std::mutex> lock(cartMutex);

        for (auto& item : items) {
            if (item.product->getId() == productId) {
                item.quantity = newQuantity;
                return true;
            }
        }

        return false;
    }

    double calculateSubtotal() const {
        std::lock_guard<std::mutex> lock(cartMutex);

        double subtotal = 0;

        for (const auto& item : items) {
            subtotal += item.product->getPrice() * item.quantity;
        }

        return subtotal;
    }

    std::vector<CartItem> getItems() const {
        std::lock_guard<std::mutex> lock(cartMutex);
        return items;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(cartMutex);
        return items.empty();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(cartMutex);
        items.clear();
    }

    void displayCart() const {
        std::lock_guard<std::mutex> lock(cartMutex);

        std::cout << "\n================ SHOPPING CART ================\n";

        if (items.empty()) {
            std::cout << "Your shopping cart is empty.\n";
            return;
        }

        for (const auto& item : items) {
            double itemTotal =
                item.product->getPrice() * item.quantity;

            std::cout << item.product->getName()
                      << " x " << item.quantity
                      << " = $" << std::fixed
                      << std::setprecision(2)
                      << itemTotal << '\n';
        }
    }
};

// ============================================================
// User Classes
// ============================================================

class User {
protected:
    std::string name;
    std::string email;
    std::string shippingAddress;
    ShoppingCart shoppingCart;

public:
    User(std::string userName,
         std::string userEmail,
         std::string userAddress)
        : name(std::move(userName)),
          email(std::move(userEmail)),
          shippingAddress(std::move(userAddress)) {

        if (name.empty() ||
            email.find('@') == std::string::npos ||
            shippingAddress.empty()) {
            throw std::invalid_argument("Invalid user information.");
        }
    }

    virtual ~User() = default;

    std::string getName() const {
        return name;
    }

    std::string getEmail() const {
        return email;
    }

    std::string getShippingAddress() const {
        return shippingAddress;
    }

    ShoppingCart& getShoppingCart() {
        return shoppingCart;
    }

    virtual std::string getMembershipType() const = 0;
    virtual double getDiscountRate() const = 0;
};

class RegularUser : public User {
public:
    using User::User;

    std::string getMembershipType() const override {
        return "Regular";
    }

    double getDiscountRate() const override {
        return 0.00;
    }
};

class PremiumUser : public User {
public:
    using User::User;

    std::string getMembershipType() const override {
        return "Premium";
    }

    double getDiscountRate() const override {
        return 0.10;
    }
};

class VIPUser : public User {
public:
    using User::User;

    std::string getMembershipType() const override {
        return "VIP";
    }

    double getDiscountRate() const override {
        return 0.15;
    }
};

// ============================================================
// Checkout Class
// ============================================================

class Checkout {
private:
    static std::mutex outputMutex;

public:
    static bool completePurchase(User& user) {
        std::vector<CartItem> cartItems =
            user.getShoppingCart().getItems();

        if (cartItems.empty()) {
            std::lock_guard<std::mutex> outputLock(outputMutex);
            std::cout << "The shopping cart is empty.\n";
            return false;
        }

        std::vector<CartItem> reservedItems;

        // Reserve each product while protecting shared inventory.
        for (const auto& item : cartItems) {
            bool reserved =
                item.product->reserveStock(item.quantity);

            if (!reserved) {
                // Restore previously reserved products.
                for (const auto& previousItem : reservedItems) {
                    previousItem.product->restoreStock(
                        previousItem.quantity
                    );
                }

                std::lock_guard<std::mutex> outputLock(outputMutex);

                std::cout << "\n" << user.getName()
                          << ": Purchase failed because "
                          << item.product->getName()
                          << " does not have enough stock.\n";

                return false;
            }

            reservedItems.push_back(item);
        }

        double subtotal =
            user.getShoppingCart().calculateSubtotal();

        double discount =
            subtotal * user.getDiscountRate();

        double finalTotal =
            subtotal - discount;

        {
            std::lock_guard<std::mutex> outputLock(outputMutex);

            std::cout << "\n============== ORDER CONFIRMATION ==============\n";
            std::cout << "Customer: "
                      << user.getName() << '\n';

            std::cout << "Membership: "
                      << user.getMembershipType() << '\n';

            std::cout << "Shipping address: "
                      << user.getShippingAddress() << '\n';

            std::cout << "\nItems purchased:\n";

            for (const auto& item : cartItems) {
                std::cout << "- "
                          << item.product->getName()
                          << " x " << item.quantity
                          << '\n';
            }

            std::cout << std::fixed << std::setprecision(2);
            std::cout << "\nSubtotal: $" << subtotal << '\n';
            std::cout << "Discount: -$" << discount << '\n';
            std::cout << "Final total: $" << finalTotal << '\n';
            std::cout << "Purchase completed successfully.\n";
            std::cout << "================================================\n";
        }

        user.getShoppingCart().clear();
        return true;
    }
};

std::mutex Checkout::outputMutex;

// ============================================================
// Input Validation Functions
// ============================================================

int readInteger(const std::string& prompt,
                int minimum,
                int maximum) {
    int number;

    while (true) {
        std::cout << prompt;

        if (std::cin >> number &&
            number >= minimum &&
            number <= maximum) {
            return number;
        }

        std::cout << "Invalid input. Enter a number from "
                  << minimum << " to " << maximum << ".\n";

        std::cin.clear();
        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n'
        );
    }
}

std::unique_ptr<User> createUser() {
    std::string name;
    std::string email;
    std::string address;

    std::cout << "Enter your name: ";
    std::getline(std::cin >> std::ws, name);

    std::cout << "Enter your email address: ";
    std::getline(std::cin, email);

    std::cout << "Enter your shipping address: ";
    std::getline(std::cin, address);

    std::cout << "\n1. Regular user - No discount\n";
    std::cout << "2. Premium user - 10% discount\n";
    std::cout << "3. VIP user - 15% discount\n";

    int membershipChoice =
        readInteger("Select your membership: ", 1, 3);

    if (membershipChoice == 2) {
        return std::make_unique<PremiumUser>(
            name, email, address
        );
    }

    if (membershipChoice == 3) {
        return std::make_unique<VIPUser>(
            name, email, address
        );
    }

    return std::make_unique<RegularUser>(
        name, email, address
    );
}

// ============================================================
// Multithreading Demonstration
// ============================================================

void runMultithreadingDemo(Catalog& catalog) {
    std::shared_ptr<Product> gameConsole =
        catalog.findProduct(4);

    if (!gameConsole) {
        std::cout << "Game console product was not found.\n";
        return;
    }

    RegularUser firstUser(
        "Concurrent User One",
        "user1@example.com",
        "LaPlace, Louisiana"
    );

    PremiumUser secondUser(
        "Concurrent User Two",
        "user2@example.com",
        "Metairie, Louisiana"
    );

    // Both users want two consoles, but only three are available.
    firstUser.getShoppingCart().addProduct(gameConsole, 2);
    secondUser.getShoppingCart().addProduct(gameConsole, 2);

    std::cout << "\nTwo users will attempt to purchase "
              << "two game consoles at the same time.\n";

    std::thread firstThread([&firstUser]() {
        Checkout::completePurchase(firstUser);
    });

    std::thread secondThread([&secondUser]() {
        Checkout::completePurchase(secondUser);
    });

    firstThread.join();
    secondThread.join();

    std::cout << "\nRemaining game console stock: "
              << gameConsole->getStockQuantity()
              << '\n';
}

// ============================================================
// Main Application
// ============================================================

int main() {
    try {
        Catalog catalog;

        catalog.addProduct(
            std::make_shared<Product>(
                1,
                "Laptop",
                "15-inch laptop computer",
                899.99,
                8
            )
        );

        catalog.addProduct(
            std::make_shared<Product>(
                2,
                "Headphones",
                "Wireless headphones",
                79.99,
                20
            )
        );

        catalog.addProduct(
            std::make_shared<Product>(
                3,
                "Smartphone",
                "128 GB smartphone",
                649.99,
                10
            )
        );

        catalog.addProduct(
            std::make_shared<Product>(
                4,
                "Game Console",
                "Current-generation game console",
                499.99,
                3
            )
        );

        std::cout << "=============================================\n";
        std::cout << "       CARTER ONLINE SHOPPING SYSTEM\n";
        std::cout << "=============================================\n\n";

        std::unique_ptr<User> currentUser = createUser();

        bool applicationRunning = true;

        while (applicationRunning) {
            std::cout << "\n================ MAIN MENU ================\n";
            std::cout << "1. View products\n";
            std::cout << "2. Add a product to the cart\n";
            std::cout << "3. Remove a product from the cart\n";
            std::cout << "4. Update a product quantity\n";
            std::cout << "5. View shopping cart\n";
            std::cout << "6. Complete checkout\n";
            std::cout << "7. Run multithreading demonstration\n";
            std::cout << "8. Exit\n";

            int menuChoice =
                readInteger("Choose an option: ", 1, 8);

            switch (menuChoice) {
                case 1: {
                    catalog.displayProducts();
                    break;
                }

                case 2: {
                    catalog.displayProducts();

                    int productId =
                        readInteger("Enter the product ID: ", 1, 100000);

                    int quantity =
                        readInteger("Enter the quantity: ", 1, 1000);

                    std::shared_ptr<Product> product =
                        catalog.findProduct(productId);

                    if (!product) {
                        std::cout << "Product not found.\n";
                    } else {
                        currentUser->getShoppingCart().addProduct(
                            product,
                            quantity
                        );

                        std::cout << product->getName()
                                  << " was added to your cart.\n";
                    }

                    break;
                }

                case 3: {
                    int productId =
                        readInteger(
                            "Enter the product ID to remove: ",
                            1,
                            100000
                        );

                    bool removed =
                        currentUser->getShoppingCart()
                            .removeProduct(productId);

                    if (removed) {
                        std::cout << "Product removed from the cart.\n";
                    } else {
                        std::cout << "Product was not found in the cart.\n";
                    }

                    break;
                }

                case 4: {
                    int productId =
                        readInteger("Enter the product ID: ", 1, 100000);

                    int newQuantity =
                        readInteger(
                            "Enter the new quantity (0 removes it): ",
                            0,
                            1000
                        );

                    bool updated =
                        currentUser->getShoppingCart()
                            .updateQuantity(productId, newQuantity);

                    if (updated) {
                        std::cout << "Shopping cart updated.\n";
                    } else {
                        std::cout << "Product was not found in the cart.\n";
                    }

                    break;
                }

                case 5: {
                    currentUser->getShoppingCart().displayCart();

                    double subtotal =
                        currentUser->getShoppingCart()
                            .calculateSubtotal();

                    double estimatedDiscount =
                        subtotal * currentUser->getDiscountRate();

                    double estimatedTotal =
                        subtotal - estimatedDiscount;

                    std::cout << std::fixed << std::setprecision(2);
                    std::cout << "Subtotal: $" << subtotal << '\n';
                    std::cout << "Membership discount: -$"
                              << estimatedDiscount << '\n';
                    std::cout << "Estimated total: $"
                              << estimatedTotal << '\n';

                    break;
                }

                case 6: {
                    if (currentUser->getShoppingCart().empty()) {
                        std::cout << "Your shopping cart is empty.\n";
                        break;
                    }

                    currentUser->getShoppingCart().displayCart();

                    std::string confirmation;

                    std::cout << "Type yes to complete the purchase: ";
                    std::cin >> confirmation;

                    if (confirmation == "yes" ||
                        confirmation == "Yes" ||
                        confirmation == "YES") {
                        Checkout::completePurchase(*currentUser);
                    } else {
                        std::cout << "The purchase was canceled.\n";
                    }

                    break;
                }

                case 7: {
                    runMultithreadingDemo(catalog);
                    break;
                }

                case 8: {
                    applicationRunning = false;
                    break;
                }
            }
        }

        std::cout << "\nThank you for visiting "
                  << "Carter Online Shopping System.\n";
    }
    catch (const std::exception& error) {
        std::cerr << "Application error: "
                  << error.what() << '\n';

        return 1;
    }

    return 0;
}
