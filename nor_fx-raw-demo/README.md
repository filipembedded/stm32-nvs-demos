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

Testovi su destruktivni za poslednja dva sektora fleša:

- sektor `4094`
- sektor `4095`

Ostatak memorije ne diraju.

## Logovanje

U projektu postoji log apstrakcija u `Core/Src/norfx_test_log.c`.
Trenutno je spremna za USB CDC transport, ali USB CDC middleware još nije prisutan u ovom demo projektu.
Kada se doda CDC device stack, dovoljno je implementirati `norfx_test_log_transport_ready()` i `norfx_test_log_transport_write()`.

## Build

```bash
cmake -S /home/filip/Documents/code/stm32-nvs-demos/nor_fx-raw-demo -B /home/filip/Documents/code/stm32-nvs-demos/nor_fx-raw-demo/build-validation
cmake --build /home/filip/Documents/code/stm32-nvs-demos/nor_fx-raw-demo/build-validation
```
