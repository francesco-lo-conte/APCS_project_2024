# Industrial Bakery Simulation

**Project for the "Algorithms and Principles of Computer Science" Course (2023-2024) at Politecnico di Milano.**

---

## Overview

[cite_start]This project is a C-based, discrete-time simulation of an industrial bakery's order management system[cite: 2]. [cite_start]It was developed to fulfill the requirements of the final exam for the "Algorithms and Data Structures" course[cite: 1].

The application simulates the bakery's complete workflow by processing a series of commands from an input file. [cite_start]It manages ingredients, recipes, warehouse inventory, customer orders, and shipping logistics, all operating within a discrete timeline where one time instant passes after each command is processed[cite: 3].

## Core Features

The simulation is built around several key entities and logical processes:

### 1. Warehouse & Inventory Management
* [cite_start]**Ingredients:** The system tracks ingredients by name[cite: 5].
* **Batches (Lotti):** The warehouse is stocked with batches of ingredients. [cite_start]Each batch has a specific quantity (in grams) and an expiration date (represented as a time instant)[cite: 9, 10].
* [cite_start]**Inventory Logic (FIFO by Expiration):** When ingredients are required for a recipe, the system **always** consumes them from the batches with the *nearest expiration date* first[cite: 14].

### 2. Recipe Management
* [cite_start]Recipes are defined by a name and a list of required ingredients with their exact quantities in grams[cite: 6, 7].
* The system allows for adding and removing recipes dynamically.

### 3. Order Processing
* **Instantaneous Preparation:** Customer orders are processed immediately. [cite_start]The system assumes preparation time is instantaneous (part of the same time instant as the command)[cite: 13].
* [cite_start]**Pending Queue (FIFO):** If the warehouse lacks sufficient ingredients to fulfill an order, the order is placed in a "pending" queue[cite: 15]. [cite_start]This queue is processed in chronological (FIFO) order; as soon as a restock provides the necessary ingredients, the oldest pending orders are fulfilled[cite: 17, 19].

### 4. Courier & Shipping Logistics
* [cite_start]**Periodic Arrival:** A courier with a fixed capacity (in grams) arrives at periodic intervals (e.g., every $n$ time instants)[cite: 20, 29].
* [cite_start]**Loading Selection (FIFO):** The courier selects from the "ready" orders in chronological order of their arrival[cite: 21].
* [cite_start]**Capacity Check:** An order is only selected if its *entire* weight fits within the courier's remaining capacity[cite: 22, 24]. [cite_start]A dessert's weight is the sum of its ingredients' weights[cite: 23].
* [cite_start]**Loading Order (Descending Weight):** After the orders for the truck have been selected, they are **loaded onto the truck in descending order of weight**[cite: 24]. [cite_start]If two orders have the same weight, they are loaded chronologically[cite: 25].

## System Commands

The simulation is driven by the following text commands:

> **`aggiungi_ricetta (recipe_name) (ingredient_name) (quantity) ...`**
>
> * Adds a new recipe to the catalog. [cite_start]It is ignored if a recipe with the same name already exists[cite: 32, 34, 35].

> **`rimuovi_ricetta (recipe_name)`**
>
> * Removes a recipe. [cite_start]This command fails if the recipe does not exist or if there are any pending or unsent orders associated with it[cite: 37, 39].

> **`rifornimento (ingredient_name) (quantity) (expiration_date) ...`**
>
> [cite_start]* Restocks the warehouse with one or more new ingredient batches[cite: 41, 43].

> **`ordine (recipe_name) (number_of_items)`**
>
> * Places a new customer order. [cite_start]It is rejected if the specified recipe does not exist[cite: 45, 47].

## Technical Constraints
* [cite_start]All names (ingredients, recipes) are alphanumeric strings (including `_`) up to 255 characters[cite: 27].
* [cite_start]All quantities, capacities, and time values are positive 32-bit integers[cite: 26, 31].

## How to Run

1.  Compile the C source files using a C compiler (e.g., GCC):
    ```bash
    gcc -o simulation *.c -Wall -std=c99
    ```
2.  Run the simulation, redirecting an input file containing the commands:
    ```bash
    ./simulation < input_file.txt
    ```
3.  [cite_start]The program will produce the simulation's output on `stdout`, including courier pickup details[cite: 48].

## Author
* **Francesco Lo Conte**
