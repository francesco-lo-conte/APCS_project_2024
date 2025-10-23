# Industrial Bakery Simulation

**Project for the "Algorithms and Principles of Computer Science" Course (2023-2024) at Politecnico di Milano.**

---

## Overview

This project is a C-based, discrete-time simulation of an industrial bakery's order management system. 
It was developed to fulfill the requirements of the final exam for the "Algorithms and Principles of Computer Science" course.

The application simulates the bakery's complete workflow by processing a series of commands from an input file. 
It manages ingredients, recipes, warehouse inventory, customer orders, and shipping logistics, all operating within a discrete timeline where one time instant passes after each command is processed.

## Core Features

The simulation is built around several key entities and logical processes:

### 1. Warehouse & Inventory Management
* **Ingredients:** The system tracks ingredients by name.
* **Batches (Lotti):** The warehouse is stocked with batches of ingredients. Each batch has a specific quantity (in grams) and an expiration date (represented as a time instant).
* **Inventory Logic (FIFO by Expiration):** When ingredients are required for a recipe, the system **always** consumes them from the batches with the *nearest expiration date* first.

### 2. Recipe Management
* Recipes are defined by a name and a list of required ingredients with their exact quantities in grams.
* The system allows for adding and removing recipes dynamically.

### 3. Order Processing
* **Instantaneous Preparation:** Customer orders are processed immediately. The system assumes preparation time is instantaneous (part of the same time instant as the command).
* **Pending Queue (FIFO):** If the warehouse lacks sufficient ingredients to fulfill an order, the order is placed in a "pending" queue. This queue is processed in chronological (FIFO) order; as soon as a restock provides the necessary ingredients, the oldest pending orders are fulfilled.

### 4. Courier & Shipping Logistics
* **Periodic Arrival:** A courier with a fixed capacity (in grams) arrives at periodic intervals (e.g., every $n$ time instants).
* **Loading Selection (FIFO):** The courier selects from the "ready" orders in chronological order of their arrival.
* **Capacity Check:** An order is only selected if its *entire* weight fits within the courier's remaining capacity. A dessert's weight is the sum of its ingredients' weights.
* **Loading Order (Descending Weight):** After the orders for the truck have been selected, they are **loaded onto the truck in descending order of weight**. If two orders have the same weight, they are loaded chronologically.

## System Commands

The simulation is driven by the following text commands:

> **`aggiungi_ricetta (recipe_name) (ingredient_name) (quantity) ...`**
>
> * Adds a new recipe to the catalog. It is ignored if a recipe with the same name already exists.

> **`rimuovi_ricetta (recipe_name)`**
>
> * Removes a recipe. This command fails if the recipe does not exist or if there are any pending or unsent orders associated with it.

> **`rifornimento (ingredient_name) (quantity) (expiration_date) ...`**
>
> * Restocks the warehouse with one or more new ingredient batches.

> **`ordine (recipe_name) (number_of_items)`**
>
> * Places a new customer order. It is rejected if the specified recipe does not exist.

## Technical Constraints
* All names (ingredients, recipes) are alphanumeric strings (including `_`) up to 255 characters.
* All quantities, capacities, and time values are positive 32-bit integers.

## How to Run

1.  Compile the C source files using a C compiler (e.g., GCC):
    ```bash
    gcc -o simulation *.c -Wall -std=c99
    ```
2.  Run the simulation, redirecting an input file containing the commands:
    ```bash
    ./simulation < input_file.txt
    ```
3.  The program will produce the simulation's output on `stdout`, including courier pickup details.

## Author
* **Francesco Lo Conte**
