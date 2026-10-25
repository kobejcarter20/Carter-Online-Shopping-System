Carter Online Shopping System
Video Demo: https://youtu.be/tKWf9ZwTQfo
Description
The Carter Online Shopping System is a console-based C++ application that simulates the major functions of an online retail platform. The project allows a customer to create a basic profile, choose a membership level, view available products, add products to a shopping cart, remove products, update quantities, review the cart, and complete a purchase. The program was designed to demonstrate object-oriented programming, multithreading, input validation, inventory management, and safe handling of shared data.
When the program begins, the customer enters a name, email address, and shipping address. The customer then selects a regular, premium, or VIP membership. Each membership type applies a different discount during checkout. A regular customer receives no discount, a premium customer receives a 10 percent discount, and a VIP customer receives a 15 percent discount. This feature demonstrates inheritance and polymorphism because each customer type shares common information and behavior while calculating discounts differently.
After registration, the program displays a menu that allows the customer to interact with the shopping system. The product catalog includes several items with identification numbers, descriptions, prices, and available stock quantities. The customer can select a product by its identification number and choose how many units to add to the cart. The program checks the requested quantity before adding an item, which prevents invalid quantities and purchases that exceed the available inventory.
The shopping cart stores the selected products and quantities. Customers can view their cart, remove products, or modify item quantities before completing checkout. The cart calculates the subtotal, membership discount, tax, and final total. During checkout, the program simulates payment processing and updates inventory after a successful purchase. If an item becomes unavailable, the inventory information prevents another customer from purchasing more units than are in stock.
Project Files
The main.cpp file contains the complete source code for the application. It includes the product, catalog, shopping cart, and user-related classes. It also contains the menu system, input-validation functions, checkout logic, inventory updates, payment simulation, and multithreading demonstration. The entire application was kept in one source file to make the project easy to compile, review, and submit while still maintaining organization through separate classes and functions.
The README.md file explains the purpose of the project, its main features, the responsibilities of each project file, important design decisions, and instructions for compiling and running the program.
The .gitignore file identifies files that Git should not track. This prevents temporary files, compiled executable files, and other unnecessary development files from being added to the repository.
Key Design Decisions
Object-oriented programming was selected because an online shopping platform naturally contains separate objects with different responsibilities. The Product class represents an individual item and stores information such as its identification number, name, description, price, and stock quantity. Encapsulation protects this information by keeping class data private and providing controlled methods for accessing or modifying it.
The catalog manages the available products, while the shopping cart manages the customer’s selections. Separating these responsibilities makes the program easier to understand, test, debug, and expand. For example, new products can be added without changing the customer classes, and new membership types can be introduced without rebuilding the shopping-cart system.
Inheritance is used to create specialized customer types from a common user structure. Polymorphism allows the application to request a discount through the same interface while producing a result based on the customer’s membership type. This reduces duplicated code and makes it easier to add another membership category later.
Smart pointers are used to manage dynamically created user objects safely. This design reduces the possibility of memory leaks because allocated memory is released automatically when it is no longer needed.
Multithreading is included to demonstrate how an online shopping system could process multiple activities concurrently. Because multiple threads may attempt to access shared inventory, mutex locks are used to protect critical sections of the program. Locking shared data helps prevent race conditions, overselling, and inconsistent stock quantities. The program keeps these protected sections limited so threads do not remain blocked longer than necessary.
Input validation was also an important design decision. Menu selections, quantities, membership choices, and payment information are checked before the program continues. Invalid input produces an understandable message instead of causing the application to stop unexpectedly. This improves reliability and provides a better user experience.
How to Compile and Run
This project requires a C++ compiler that supports C++17 and threading. From the terminal, navigate to the folder containing main.cpp and compile the program with:
g++ -std=c++17 -pthread main.cpp -o online_shop
Run the compiled application with:
./online_shop
The user can then follow the on-screen prompts to create a customer profile, select a membership, shop for products, manage the cart, complete checkout, and view the multithreading demonstration.
Author
Kobe Carter
 GitHub: kobejcarter20
 LaPlace, Louisiana, United States
 October 25, 2026

