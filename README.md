# Industrial Bakery Simulation

**Project for the "Algorithms and Principles of Computer Science" Course (2023-2024) at Politecnico di Milano.**

---

## Overview

This project is a C-based, discrete-time simulation of an industrial bakery's order management system. It was developed to fulfill the requirements of the final exam for the "Algorithms and Principles of Computer Science" course.

The application simulates the bakery's complete workflow by processing a series of commands from an input file. It manages ingredients, recipes, warehouse inventory, customer orders, and shipping logistics, all operating within a discrete timeline where one time instant passes after each command is processed.

## Core Features

The simulation is built around several key entities and logical processes:

* **Warehouse & Inventory Management:** The system tracks ingredients by name. The warehouse is stocked with batches, each with a specific quantity and an expiration date.
* **Inventory Logic (FIFO by Expiration):** When ingredients are required, the system **always** consumes them from the batches with the *nearest expiration date* first.
* **Recipe Management:** Recipes are defined by a name and a list of required ingredients with their exact quantities.
* **Order Processing (FIFO Queue):** Customer orders are processed immediately. If the warehouse lacks sufficient ingredients, the order is placed in a "pending" queue. This queue is processed in chronological (FIFO) order as soon as a restock provides the necessary ingredients.
* **Courier & Shipping Logistics:** A courier with a fixed capacity arrives periodically.
    1.  **Selection:** Orders are selected from the "ready" queue chronologically (FIFO) until the truck is full.
    2.  **Loading:** The selected orders are then **loaded onto the truck in descending order of weight**.

---

## Technical Design & Data Structures

To meet the project's efficiency requirements, specific data structures were chosen:

* **Hash Tables for $O(1)$ Lookups:**
    Recipes and ingredients are stored in **hash tables** (`ingredienti_head` and `ricette_head`). This provides an average-case $O(1)$ time complexity for searching, adding, or retrieving items by name, which is critical for performance and vastly superior to a linear $O(N)$ scan. The implementation uses *separate chaining* with doubly-linked lists for collision handling.

* **Doubly-Linked Lists for Dynamic Data:**
    All dynamic collections—such as order queues and ingredient batches—are implemented as **doubly-linked lists**. This allows for efficient $O(1)$ insertion and removal of nodes as orders change state (e.g., from `pending` to `ready`) or as ingredient batches are consumed.

* **Efficient Expiration Logic (Sorted List):**
    The "FIFO by Expiration" requirement is implemented efficiently. Each ingredient maintains a list of its available batches (`rifornimenti_head`). This list is **kept sorted by expiration date** at all times. When a restock occurs, the new batch is inserted in its correct sorted position. This design makes consumption trivial: the system simply takes from the **head of the list**, which is guaranteed to be the batch with the nearest expiration date.

* **Order Queue Management:**
    * **Pending Queue (`ordine_in_attesa_head`):** A list managed as a FIFO queue. After a restock, the system iterates this queue from tail to head to process the oldest pending orders first.
    * **Ready Queue (`ordine_pronti_head`):** A list kept **sorted by arrival time** to ensure the courier selects orders chronologically.

* **Performance Optimizations:**
    * **Custom Input Parser:** A custom buffered parser (`prossima_input`) is used for reading commands, which is significantly faster than repeated calls to standard `stdio` functions like `scanf`.
    * **Results Caching:** The system caches calculated values (e.g., total available grams per ingredient, max production capacity per recipe within a time instant) to avoid redundant computations.

---

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
