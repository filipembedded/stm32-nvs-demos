# nor_fx-raw-demo

STM32F4 demo koji pokreće sekvencijalni self-test za `nor_fx` biblioteku na realnom SPI NOR uređaju.

## Šta testira

- `norfx_reset`
- `norfx_read_id`
- `norfx_read_status_reg`
- `norfx_write_enable` i `norfx_write_disable`
- `norfx_read` i `norfx_fast_read`
- `norfx_erase_sector`
- `norfx_page_program`
- `norfx_write`

## Test region

Testovi su sada **FULL / destruktivni za ceo flash**:

- sektor `0`
- ...
- sektor `4095`

To znači da demo briše, programira i verifikuje **sve sektore** spoljnog NOR fleša.

> Ako na čipu postoji bilo kakav važan sadržaj, ovaj demo će ga obrisati.

Pored full sweep-a, demo radi i reprezentativne testove za:

- cross-page read
- NOR `1 -> 0` behavior
- `norfx_write()` read-modify-write
- cross-sector write preko granice sektora `0 -> 1`

## Logovanje

U projektu postoji log apstrakcija u `Core/Src/norfx_test_log.c`.
Logovanje je povezano na USB CDC i self-test ispisuje progres i rezultat preko virtuelnog COM porta.

Tokom full sweep-a ispisuje se jedna linija za svaki sektor sa vremenima za:

- erase
- blank read
- write
- verify read
- ukupan prolaz sektora

Na kraju testa ispisuje se i rezime performansi za:

- čitanje (`read`)
- upis (`write`)
- brisanje (`erase`)

Vremena se mere na targetu sa mikrosekundskom rezolucijom kada je dostupan DWT cycle counter.

## Build

```bash
cmake -S /home/filip/Documents/code/stm32-nvs-demos/nor_fx-raw-demo -B /home/filip/Documents/code/stm32-nvs-demos/nor_fx-raw-demo/build-validation
cmake --build /home/filip/Documents/code/stm32-nvs-demos/nor_fx-raw-demo/build-validation
```
