#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT(x) \
  if (!(x)) { \
    printf("ASSERT (%s) failed %d\n", #x, __LINE__); \
    exit(0); \
  }

#define INSERT_NODE_HEAD(node, head) \
  node->next = head; \
  node->prev = NULL; \
  if (head) { \
    head->prev = node; \
  } \
  head = node;

#define INSERT_NODE_ORDERED(node, head, nodecmp) \
  if (nodecmp(node, head) == -1) { \
    node->next = head; \
    node->prev = NULL; \
  } else { \
    node->prev = head; \
    while (1) { \
      node->next = node->prev->next; \
      if (nodecmp(node, node->next) == -1) { \
        break; \
      } \
      node->prev = node->next; \
    } \
    node->next = node->prev->next; \
  } \
  if (node->prev) { \
    node->prev->next = node; \
  } else { \
    head = node; \
  } \
  if (node->next) { \
    node->next->prev = node; \
  }

#define REMOVE_NODE(node, head) \
    if (node->prev) { \
      node->prev->next = node->next; \
    } else { \
      head = node->next; \
    } \
    if (node->next) { \
      node->next->prev = node->prev; \
    } \
    node->prev = NULL; \
    node->next = NULL;

#define HASH_BUCKETS 1024
size_t hash_string(const char* str) {
  size_t result = 12345;
  int len = strlen(str);
  for (int i = 0; i < len; ++i) {
    result <<= 5;
    result ^= str[i] + 7;
    result %= HASH_BUCKETS;
  }
  return result;
}

struct Ingrediento* ingredienti_head[HASH_BUCKETS] = {NULL};
struct Ricetta* ricette_head[HASH_BUCKETS] = {NULL};
struct Ordine* ordine_in_attesa_head = NULL;
struct Ordine* ordine_pronti_head = NULL;

int corriere_periodicita = 0;
int camioncino_capacita = 0;
int istante = 0;

struct Rifornimento {
  int grammi;
  int scadenza;
  struct Rifornimento* prev;
  struct Rifornimento* next;
};

struct Rifornimento* rifornimento_alloc(int grammi, int scadenza) {
  struct Rifornimento* r = (struct Rifornimento*)malloc(sizeof(struct Rifornimento));
  r->grammi = grammi;
  r->scadenza = scadenza;
  return r;
}

void rifornimento_free(struct Rifornimento* r) {
  ASSERT(r->prev == NULL);
  ASSERT(r->next == NULL);
  free(r);
}

struct Ingrediento {
  char* name;
  struct Rifornimento* rifornimenti_head;
  int grammi_totali;
  int istante_scaduti_buttato;
  struct Ingrediento* prev;
  struct Ingrediento* next;
};

struct Ingrediento* ingrediento_alloc(const char* name) {
  struct Ingrediento* i = (struct Ingrediento*)malloc(sizeof(struct Ingrediento));
  i->name = strdup(name);
  i->grammi_totali = 0;
  i->istante_scaduti_buttato = -1;
  i->rifornimenti_head = NULL;
  i->prev = NULL;
  i->next = NULL;
  return i;
}

void ingrediento_free(struct Ingrediento* i) {
  ASSERT(i->prev == NULL);
  ASSERT(i->next == NULL);
  free(i->name);
  free(i);
}

struct Ingrediento* ingrediento_prendere(const char* name) {
  if (name == NULL) {
    return NULL;
  }
  size_t hash = hash_string(name);
  for (struct Ingrediento* i = ingredienti_head[hash]; i; i = i->next) {
    if (!strcmp(i->name, name)) {
      return i;
    }
  }
  struct Ingrediento* i = ingrediento_alloc(name);
  INSERT_NODE_HEAD(i, ingredienti_head[hash]);
  return i;
}

void ingrediento_butta_scaduti(struct Ingrediento* i) {
  if (i->istante_scaduti_buttato == istante) {
    return;
  }
  i->istante_scaduti_buttato = istante;

  struct Rifornimento* r_next = NULL;
  for (struct Rifornimento* r = i->rifornimenti_head; r; r = r_next) {
    r_next = r->next;
    if (r->scadenza <= istante) {
      i->grammi_totali -= r->grammi;
      REMOVE_NODE(r, i->rifornimenti_head);
      rifornimento_free(r);
    } else {
      break;
    }
  }
}

struct RicettaIngrediento {
  struct Ingrediento* ingrediento;
  int grammi;
  struct RicettaIngrediento* prev;
  struct RicettaIngrediento* next;
};

struct Ricetta {
  char* name;
  int grammi_totali;

  int num_ordini_in_attesa;
  int num_ordini_pronti;
  int capacita_istante;
  int capacita;

  struct RicettaIngrediento* ingredienti_head;
  struct Ricetta* prev;
  struct Ricetta* next;
};

struct Ricetta* ricetta_alloc_insert(const char* name) {
  struct Ricetta* r = (struct Ricetta*)malloc(sizeof(struct Ricetta));
  r->name = strdup(name);
  r->grammi_totali = 0;
  r->num_ordini_in_attesa = 0;
  r->num_ordini_pronti = 0;
  r->capacita = 0;
  r->capacita_istante = -1;
  r->ingredienti_head = NULL;
  r->prev = NULL;
  r->next = NULL;

  size_t hash = hash_string(name);
  INSERT_NODE_HEAD(r, ricette_head[hash]);
  return r;
}

void ricetta_remove_free(struct Ricetta* r) {
  size_t hash = hash_string(r->name);
  REMOVE_NODE(r, ricette_head[hash]);

  struct RicettaIngrediento* ri_next = NULL;
  for (struct RicettaIngrediento* ri = r->ingredienti_head; ri; ri = ri_next) {
    ri_next = ri->next;
    free(ri);
  }
  ASSERT(r->prev == NULL);
  ASSERT(r->next == NULL);
  free(r->name);
  free(r);
}

void ricetta_aggungia_ingrediento(struct Ricetta* r, struct Ingrediento* i, int grammi) {
  struct RicettaIngrediento* ri = (struct RicettaIngrediento*)malloc(sizeof(struct RicettaIngrediento));
  ri->ingrediento = i;
  ri->grammi = grammi;
  r->grammi_totali += grammi;
  INSERT_NODE_HEAD(ri, r->ingredienti_head);
}

struct Ricetta* ricetta_cerca(const char* name) {
  size_t hash = hash_string(name);
  for (struct Ricetta* ricetta = ricette_head[hash]; ricetta; ricetta = ricetta->next) {
    if (!strcmp(ricetta->name, name)) {
      return ricetta;
    }
  }
  return NULL;
}

struct Ordine {
  struct Ricetta* ricetta;
  int quantita;
  int grammi;
  int instante_acetato;
  struct Ordine* prev;
  struct Ordine* next;
};

struct Ordine* ordine_alloc(struct Ricetta* ricetta, int quantita) {
  struct Ordine* o = (struct Ordine*)malloc(sizeof(struct Ordine));
  o->ricetta = ricetta;
  o->quantita = quantita;
  o->instante_acetato = istante;
  o->grammi = ricetta->grammi_totali * o->quantita;
  return o;
}

void ordine_free(struct Ordine* o) {
  ASSERT(o->prev == NULL);
  ASSERT(o->next == NULL);
  free(o);
}

int ordine_cmp_entrare_camioncino(struct Ordine* a, struct Ordine* b) {
  if (a && !b) {
    return -1;
  }
  if (b && !a) {
    return 1;
  }
  if (!b && !a) {
    return 0;
  }
  // Poi quello acetato preima va prima
  if (a->instante_acetato < b->instante_acetato) {
    return -1;
  }
  if (a->instante_acetato > b->instante_acetato) {
    return 1;
  }
  return 0;
}

int ordine_cmp_uscire_camioncino(struct Ordine* a, struct Ordine* b) {
  if (a && !b) {
    return -1;
  }
  if (b && !a) {
    return 1;
  }
  if (!b && !a) {
    return 0;
  }
  if (a->grammi > b->grammi) {
    return -1;
  }
  if (a->grammi < b->grammi) {
    return 1;
  }
  if (a->instante_acetato < b->instante_acetato) {
    return -1;
  }
  if (a->instante_acetato > b->instante_acetato) {
    return 1;
  }
  return 0;
}

int ordine_prova_preparare(struct Ordine* o) {
  struct Ricetta* r = o->ricetta;

  if (r->capacita_istante == istante && r->capacita < o->quantita) {
    return 0;
  }

  int capacita = -1;
  for (struct RicettaIngrediento* ri = r->ingredienti_head; ri; ri = ri->next) {
    struct Ingrediento* i = ri->ingrediento;
    ingrediento_butta_scaduti(i);

    int i_capacita = i->grammi_totali / ri->grammi;
    if (capacita == -1 || i_capacita < capacita) {
      capacita = i_capacita;
    }
    if (capacita == 0) {
      break;
    }
  }
  r->capacita = capacita;
  r->capacita_istante = istante;
  if (o->quantita <= r->capacita) {
    return 1;
  }
  return 0;
}

void ordine_prepara(struct Ordine* o) {
  if (!ordine_prova_preparare(o)) {
    return;
  }

  for (struct RicettaIngrediento* ri = o->ricetta->ingredienti_head; ri; ri = ri->next) {
    int grammi_voluto = ri->grammi * o->quantita;
    struct Ingrediento* i = ri->ingrediento;

    ASSERT(grammi_voluto <= i->grammi_totali);
    
    struct Rifornimento* r_next = NULL;
    for (struct Rifornimento* r = i->rifornimenti_head; r; r = r_next) {
      r_next = r->next;

      ASSERT(r->scadenza > istante);

      int grammi_preso = 0;
      if (grammi_voluto > r->grammi) {
        grammi_preso = r->grammi;
      } else {
        grammi_preso = grammi_voluto;
      }
      grammi_voluto -= grammi_preso;
      r->grammi -= grammi_preso;
      i->grammi_totali -= grammi_preso;

      if (r->grammi == 0) {
        i->grammi_totali -= r->grammi;
        REMOVE_NODE(r, i->rifornimenti_head);
        rifornimento_free(r);
        continue;
      }

      if (grammi_voluto == 0) {
        break; 
      }
    }
    ASSERT(grammi_voluto == 0);
  }
  o->ricetta->capacita -= o->quantita;

  REMOVE_NODE(o, ordine_in_attesa_head);
  o->ricetta->num_ordini_in_attesa -= 1;
  ASSERT(o->ricetta->num_ordini_in_attesa >= 0);

  INSERT_NODE_ORDERED(o, ordine_pronti_head, ordine_cmp_entrare_camioncino);
  o->ricetta->num_ordini_pronti += 1;
}

char* prossima_input() {
  static char input[256];
  static int at_eol = 0;
  int i = 0;

  if (at_eol) {
    at_eol = 0;
    return NULL;
  }

  while (1) {
    int c = 0;
    {
      static char buffer[256];
      static int buffer_n = 0;
      static int buffer_i = 0;
      if (buffer_i == buffer_n) {
        buffer_i = 0;
        buffer_n = fread(buffer, 1, 256, stdin);
      }
      if (buffer_n == 0) {
        c = EOF;
      } else {
        c = buffer[buffer_i++];
      }
    }

    // Aggiungere c ad input
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        (c == '_')) {
      input[i++] = c;
      ASSERT(i < 256);
      continue;
    }

    // Parola finita, break.
    input[i] = 0;
    if (c == '\n' || c == EOF) {
      at_eol = 1;
    }
    break;
  }
  if (i == 0) {
    return NULL;
  }
  
  return input;
}

void command_aggiungi_ricetta() {
  char* ricetta_name = prossima_input();
  if (ricetta_cerca(ricetta_name)) {
    printf("ignorato\n");
    
    while(prossima_input());
    return;
  }

  struct Ricetta* r = ricetta_alloc_insert(ricetta_name);

  while (1) {
    struct Ingrediento* i = ingrediento_prendere(prossima_input());
    if (!i) {
      break;
    }
    int grammi = atoi(prossima_input());
    ricetta_aggungia_ingrediento(r, i, grammi);
  }
  printf("aggiunta\n");
}

void command_rimuovi_ricetta() {
  struct Ricetta* r = ricetta_cerca(prossima_input());
  ASSERT(prossima_input() == NULL);
  if (!r) {
    printf("non presente\n");
    return;
  }

  if (r->num_ordini_in_attesa > 0) {
    printf("ordini in sospeso\n");
    return;
  }
  if (r->num_ordini_pronti > 0) {
    printf("ordini in sospeso\n");
    return;
  }
  ricetta_remove_free(r);
  printf("rimossa\n");
}

void command_ordine() {
  struct Ricetta* ricetta = ricetta_cerca(prossima_input());
  int quantita = atoi(prossima_input());
  ASSERT(prossima_input() == NULL);

  if (!ricetta) {
    printf("rifiutato\n");
    return;
  }

  struct Ordine* o = ordine_alloc(ricetta, quantita);

  INSERT_NODE_HEAD(o, ordine_in_attesa_head);
  o->ricetta->num_ordini_in_attesa += 1;
  ordine_prepara(o);
  printf("accettato\n");
}

void command_rifornimento() {
  while (1) {
    struct Ingrediento* i = ingrediento_prendere(prossima_input());
    if (!i) {
      break;
    }
    int grammi = atoi(prossima_input());
    int scadenza = atoi(prossima_input());
    i->grammi_totali += grammi;

    struct Rifornimento* r_prev = i->rifornimenti_head;
    if (!r_prev || scadenza < r_prev->scadenza) {
      r_prev = NULL;
    } else {
      while (r_prev->next && r_prev->next->scadenza < scadenza) {
        r_prev = r_prev->next;
      }
    }

    if (r_prev && r_prev->scadenza == scadenza) {
      r_prev->grammi += grammi;
    } else {
      struct Rifornimento* r = rifornimento_alloc(grammi, scadenza);
      r->prev = r_prev;
      if (r->prev) {
        r->next = r->prev->next;
        r->prev->next = r;
      } else {
        r->next = i->rifornimenti_head;
        i->rifornimenti_head = r;
      }
      if (r->next) {
        r->next->prev = r;
      }
    }
  }

  struct Ordine* o = ordine_in_attesa_head;
  while (o && o->next) {
    o = o->next;
  }
  while (o) {
    struct Ordine* o_prev = o->prev;
    ordine_prepara(o);
    o = o_prev;
  }
  
  printf("rifornito\n");
} 

void corriere() {
  struct Ordine* ordine_in_camioncino_head = NULL;

  {
    int camioncino_grammi = 0;
    struct Ordine* o_next = NULL;
    for (struct Ordine* o = ordine_pronti_head; o; o = o_next) {
      o_next = o->next;
      if (camioncino_grammi + o->grammi > camioncino_capacita) {
        break;
      }
      camioncino_grammi += o->grammi;
      REMOVE_NODE(o, ordine_pronti_head);
      INSERT_NODE_ORDERED(o, ordine_in_camioncino_head, ordine_cmp_uscire_camioncino);
    }
  }

  if (ordine_in_camioncino_head == NULL) {
    printf("camioncino vuoto\n");
    return;
  }

  {
    struct Ordine* o_next = NULL;
    for (struct Ordine* o = ordine_in_camioncino_head; o; o = o_next) {
      o_next = o->next;
      printf("%d %s %d\n",
        o->instante_acetato,
        o->ricetta->name,
        o->quantita);
 
      o->ricetta->num_ordini_pronti -= 1;
      REMOVE_NODE(o, ordine_in_camioncino_head);
      ordine_free(o);
    }
  }
}

int main(int argc, char* argv[]) {
  corriere_periodicita = atoi(prossima_input());
  camioncino_capacita = atoi(prossima_input());
  prossima_input();

  while (1) {
    char* command = prossima_input();
    if (!command) {
      break;
    }

    if (!strcmp(command, "aggiungi_ricetta")) {
      command_aggiungi_ricetta();
    } else if (!strcmp(command, "rimuovi_ricetta")) {
      command_rimuovi_ricetta();
    } else if (!strcmp(command, "ordine")) {
      command_ordine();
    } else if (!strcmp(command, "rifornimento")) {
      command_rifornimento();
    } else {
      ASSERT(!"command invalido");
    }

    istante += 1;
    if (istante % corriere_periodicita == 0) {
      corriere();
    }
  }
  return 0;
}

