# Arduino
Przerwania timerów w mikrokontrolerze ATmega328P
![obraz](https://github.com/user-attachments/assets/bdbaa695-b105-4974-a53d-a3e7b6109393)
8-bit timer:
![obraz](https://github.com/user-attachments/assets/e7f3b9a2-e8de-42c7-a6c2-6525d2c42cb7)


Mikrokontroler ATmega328P posiada trzy wewnętrzne timery/countery: Timer0 (8-bitowy), Timer1 (16-bitowy) oraz Timer2 (8-bitowy). Każdy timer może generować przerwania w kilku trybach: po przepełnieniu licznika (Overflow), po dopasowaniu wartości licznika do rejestru porównawczego (Compare Match A/B) oraz (w przypadku Timer1) po zarejestrowaniu zewnętrznego sygnału w kanale Input Capture. Timery mają niezależne preskalery i rejestry porównawcze, co pozwala na elastyczne generowanie przerwań o różnej częstotliwości. Poniżej omówiono każdy timer osobno.
Timer0 (8-bit)

    Tryby przerwań: Timer0 obsługuje przepełnienie (TOV0), dopasowanie do OCR0A (OCF0A) i dopasowanie do OCR0B (OCF0B). Nie ma trybu Input Capture. Przerwania generowane są, gdy licznik TCNT0 zrzuci się z 0xFF do 0 (overflow) lub gdy osiągnie wartość zarejestrowaną w OCR0A lub OCR0B (compare match).

    Rejestry konfiguracyjne:

        TCCR0A, TCCR0B – ustawiają tryb pracy (w tym bity WGM0x dla trybów normalnego/CTC/PWM) oraz preskaler (bity CS02:0 w TCCR0B).

        OCR0A, OCR0B – rejestry porównania 8-bitowego.

        TIMSK0 – maska przerwań Timer0: bity OCIE0B, OCIE0A, TOIE0 (bit2,1,0) włączają lokalnie przerwania porównań B, A i przepełnienia.

        TIFR0 – flaga przerwań: OCF0A, OCF0B, TOV0 (bit2,1,0) wskazują wystąpienie zdarzenia (czyszczone sprzętowo po obsłudze ISR lub przez zapis jedynki).

    Preskaler i wartość porównania: Preskaler wybieramy przez ustawienie bitów CS02, CS01, CS00 w rejestrze TCCR0B, np. TCCR0B |= (1<<CS01) dla preskalera 8. Dzięki temu zegar licznika to zegar CPU podzielony przez wybrany czynnik (1, 8, 64, 256, 1024). Wartość, przy której ma nastąpić przerwanie Compare Match, zapisuje się w OCR0A lub OCR0B. Przykładowo, aby uzyskać przerwanie co N taktów przy preskalerze, wpisujemy OCR0A = (wartość). Na przykład, dla F_CPU=16 MHz, preskalera 1024 i przerwania co ok. 1 ms w trybie CTC, można obliczyć OCR0A = (16e6/1024/1000) - 1 ≈ 15.

    Procedura obsługi przerwania (ISR): Aby obsłużyć przerwanie, w kodzie C definiujemy funkcję ISR z odpowiednim wektorem, np.:

ISR(TIMER0_OVF_vect) {
    // kod wykonywany po przepełnieniu Timer0
}
ISR(TIMER0_COMPA_vect) {
    // kod wykonywany po dopasowaniu do OCR0A
}
ISR(TIMER0_COMPB_vect) {
    // kod wykonywany po dopasowaniu do OCR0B
}

Po wejściu do ISR sprzętowo jest czyszczona flaga TOV0/OCF0x i wykonywany jest nasz kod. Po zakończeniu ISR wykonywana jest instrukcja RETI, która przywraca flagę globalnego włącznika przerwań i pozwala na kolejne przerwania.

Włączanie przerwań: Lokalnie włączamy przerwanie, zapisując 1 do odpowiedniego bitu w TIMSK0, np. TIMSK0 |= (1<<TOIE0) dla przerwania przepełnienia. Należy również włączyć przerwania globalnie przez ustawienie bitu I w rejestrze SREG – w praktyce wykonuje się to funkcją sei() z <avr/interrupt.h>. Dzięki temu wszystkie włączone przerwania Timer0 będą mogły zadziałać (bit I w SREG musi być 1
ww1.microchip.com
).

Przykładowy kod (miganie diodą): Konfigurujemy Timer0 w trybie normalnym, preskaler 64 i włączamy przerwanie overflow. W ISR przepełnienia czyścimy licznik i zmieniamy stan pinu. Przykład (F_CPU=8MHz, preskaler 64, LED na PB0):

    #include <avr/io.h>
    #include <avr/interrupt.h>

    int main(void) {
        DDRB |= (1<<PB0);        // PB0 wyjściem (LED)
        TCNT0 = 0;               // licznik na 0
        TCCR0A = 0x00;           // tryb normalny (WGM01:0 = 00)
        TCCR0B = (1<<CS01)|(1<<CS00); // preskaler = 64 (CS02:0 = 011)
        TIMSK0 |= (1<<TOIE0);    // włącz przerwanie przepełnienia
        sei();                   // włącz przerwania globalnie
        while(1) {
            // główna pętla pusta, wszystko robi ISR
        }
    }
    ISR(TIMER0_OVF_vect) {
        // np. co ~1ms (8MHz/64/256 ~ 488Hz)
        PORTB ^= (1<<PB0);      // toggle LED
        // TOV0 sprzętowo wyzerowany
    }

    Kod ten migota diodą co pewien czas ustalany częstotliwością przepełnień licznika. Innym przykładem jest tryb CTC, gdzie w ISR użyjemy wektora TIMER0_COMPA_vect po dopasowaniu do OCR0A, co pozwala na precyzyjne opóźnienia.

Timer1 (16-bit)

    Tryby przerwań: Timer1 ma przerwania przepełnienia (TOV1), porównań OCR1A (OCF1A) i OCR1B (OCF1B) oraz dodatkowo przerwanie Input Capture (ICF1) po zarejestrowaniu na pinie ICP1 (PD6) ustawionego zbocza sygnału. W ten sposób można mierzyć czasy trwania sygnałów zewnętrznych lub generować przerwania w oparciu o dwubajtowy licznik.

    Rejestry konfiguracyjne:

        TCCR1A, TCCR1B, TCCR1C – ustawiają tryb licznika (bity WGM13:0, FOC1A/B, COM1A/B itp.) oraz preskaler (CS12, CS11, CS10 w TCCR1B).

        OCR1A, OCR1B – 16-bitowe rejestry porównania.

        ICR1 – 16-bitowy rejestr Input Capture (może służyć jako dolna/pełna granica trybów PWM lub rejestr porównania przy pewnych WGM).

        TIMSK1 – maska przerwań Timer1: bity ICIE1 (bit5, włączający przerwanie Input Capture), OCIE1B (bit2, Compare B), OCIE1A (bit1, Compare A) i TOIE1 (bit0, Overflow).

        TIFR1 – flagi przerwań: ICF1, OCF1B, OCF1A, TOV1. Flagi są zerowane po obsłudze ISR.

    Preskaler i wartość porównania: Podobnie jak w Timer0, preskaler wybiera się przez bity CS12:0. W trybie CTC (np. WGM12=1) ustawiamy przerwanie po dopasowaniu do OCR1A. Przykład: aby uzyskać przerwanie 1 Hz z zegarem 16MHz, można ustawić OCR1A = 15624 i preskaler 1024 (bo 16e6/1024/ (15624+1) ≈ 1). W trybie Capture (ICR1), przerwanie ICF1 wystąpi przy zewnętrznym zdarzeniu na pinie ICP1.

    Procedura obsługi przerwania (ISR): Definiujemy ISR-y:

ISR(TIMER1_OVF_vect) { /* przepełnienie 16-bitowego licznika */ }
ISR(TIMER1_COMPA_vect) { /* dopasowanie do OCR1A */ }
ISR(TIMER1_COMPB_vect) { /* dopasowanie do OCR1B */ }
ISR(TIMER1_CAPT_vect) { /* przechwycenie sygnału (ICF1) */ }

Wektor TIMER1_CAPT_vect wywoływany jest po ustawieniu bitu ICF1 (zachowa wartość licznika w ICR1). Flaga ICF1 zostaje wyzerowana sprzętowo po wejsciu do ISR.

Włączanie przerwań: Analogicznie, ustawiamy bity w TIMSK1: TIMSK1 |= (1<<OCIE1A) dla Compare A, | (1<<TOIE1) dla przepełnienia, | (1<<ICIE1) dla Input Capture. Przykładowo, kod z QEEWiki ustawia przerwanie Compare A i preskaler 1024:

    OCR1A = 15624;
    TCCR1B |= (1<<WGM12);       // tryb CTC (WGM12=1)
    TIMSK1 |= (1<<OCIE1A);      // włącz przerwanie Compare A
    TCCR1B |= (1<<CS12)|(1<<CS10); // preskaler 1024
    sei();                      // włącz globalnie przerwania

    ISR w QEEWiki: ISR(TIMER1_COMPA_vect) { /* akcja co 1s */ }.

    Przykład użycia: Timer1 jest często stosowany do precyzyjnych zegarów czasu rzeczywistego lub PWM. Przykład: generowanie przerwania co 1 s i przełączanie LED (kod jak wyżej). Innym zastosowaniem jest pomiar impulsu na pinie ICP1 – konfigurujemy TCCR1B |= (1<<ICES1) dla zbocza rosnącego, włączamy ICIE1 i czytamy wartość ICR1 w ISR TIMER1_CAPT_vect.

Timer2 (8-bit)

    Tryby przerwań: Timer2, podobnie jak Timer0, oferuje przerwania przepełnienia (TOV2) oraz porównań OCR2A (OCF2A) i OCR2B (OCF2B). Nie ma Input Capture. Dodatkowo Timer2 może pracować asynchronicznie z zewnętrznym oscylatorem (np. do zegara rzeczywistego).

    Rejestry konfiguracyjne:

        TCCR2A, TCCR2B – tryb pracy (bity WGM22:0) i preskaler (CS22:0 w TCCR2B).

        OCR2A, OCR2B – rejestry porównań 8-bit.

        TIMSK2 – maska przerwań Timer2: OCIE2B (bit2), OCIE2A (bit1), TOIE2 (bit0).

        TIFR2 – flagi OCF2B, OCF2A, TOV2.

    Preskaler i wartość porównania: Preskaler Timer2 ustawiamy bitami CS22:0 (możliwe opcje: 1,8,32,64,128,256,1024). OCR2A/B określają moment wygenerowania przerwania w trybach CTC/PWM. Na przykład, przy F_CPU=16MHz, preskalerze 1024 i trybie normalnym, przepełnienie co 65536 taktów nastąpi ≈38ms, co by dawało częstotliwość ~26Hz.

    Procedura obsługi przerwania (ISR): Podobnie do Timer0, ISR-y to ISR(TIMER2_OVF_vect), ISR(TIMER2_COMPA_vect), ISR(TIMER2_COMPB_vect). W wnętrzu ISR można np. przełączać stan pinu lub składować wynik.

    Włączanie przerwań: Przykład włączenia przerwania porównania A: TIMSK2 |= (1<<OCIE2A). Potem sei().

    Przykład: Timer2 często służy do krótkich opóźnień lub PWM. Dla przykładu, w trybie CTC możemy robić przerwanie co np. 1ms ustawiając TCCR2A |= (1<<WGM21); OCR2A = 249; TCCR2B |= (1<<CS22)|(1<<CS20); (preskaler 1024, F_CPU 16MHz daje 16MHz/1024/250 ≈ 62.5Hz, czyli przerwanie co 16ms – analogicznie można dobrać). W ISR co jeden odliczamy dziesiętne sekundy itp. Można też wykorzystać przerwania Timer2 do generowania przebiegu PWM: w trybie Fast PWM ustawiamy COM2A1:0, WGM21:0 i OCR2A, a ISR(TIMER2_OVF_vect) może np. przetwarzać zakończony okres.

Inne przerwania wewnętrzne w ATmega328P

Oprócz przerwań timerów mikrokontroler oferuje wiele innych wewnętrznych źródeł przerwań:

    ADC – przetwornik analogowo-cyfrowy: Po zakończeniu konwersji ADC ustawiana jest flaga ADIF w rejestrze ADCSRA. Jeśli bit ADIE (ADC Interrupt Enable) jest ustawiony oraz globalny bit I w SREG = 1, generowane jest przerwanie ADC_vect. W ten sposób można wykonywać kolejne konwersje cyklicznie i odczytywać wynik w ISR (flagi ADIF czyszczone automatycznie). Służy to do pomiaru napięcia z czujników itp.

    USART (UART): Port szeregowy generuje kilka przerwań: Receive Complete (RXC) – gdy pojawi się nowy bajt w UDR0; Data Register Empty (UDRE) – gdy bufor TX jest pusty; Transmit Complete (TXC) – po zakończeniu wysyłania bajtu. Odpowiednie bity RXCIE0, UDRIE0, TXCIE0 w rejestrze UCSR0B włączają te przerwania. Po wystąpieniu ISR czyta się UDR0 lub pisze kolejną daną do wysłania. Pozwala to na asynchroniczną obsługę komunikacji szeregowej (UART).

    External INT0/INT1: Dwa zewnętrzne przerwania na pinach PD2 (INT0) i PD3 (INT1). Można je zaprogramować na wyzwalanie przy zboczu opadającym/rosnącym lub poziomie niskim/zmianie poziomu, ustawiając bity ISC01..0 i ISC11..10 w rejestrze EICRA. Gdy bit INT0 lub INT1 w EIMSK zostanie ustawiony oraz globalny bit I = 1, pojawi się odpowiedni wektor INT0_vect lub INT1_vect. Służą do reagowania na zewnętrzne zdarzenia (np. naciśnięcie przycisku) z bardzo niskim czasem reakcji.

    Pin Change Interrupts (PCINT0..PCINT23): Przerwania zmian stanów na wielu pinach GPIO. Piny są pogrupowane w trzy bloki PCINT23..16, PCINT15..8, PCINT7..0. W rejestrze PCICR ustawiamy bity PCIE2, PCIE1, PCIE0 aby włączyć przerwania tych grup. Dodatkowo w maskach PCMSK2/1/0 zaznaczamy, które piny z danej grupy mają zgłaszać przerwanie. Jeśli na włączonym pinie nastąpi zmiana logiczna, generowane jest przerwanie PCI2, PCI1 lub PCI0. Na przykład PCICR |= (1<<PCIE0); PCMSK0 |= (1<<PCINT0); włączy przerwanie zmiany stanu pinu PB0. ISR dla takiego przerwania to PCINT0_vect, PCINT1_vect lub PCINT2_vect, zależnie od grupy.

    TWI (I²C Master/Slave): Wbudowany interfejs TWI (I²C) generuje przerwania po realizacji każdego kroku protokołu (np. zakończenie wysyłania/odbioru bajtu). W rejestrze TWCR bit TWIE włącza przerwanie TWI, które następuje, gdy ustawiony jest bit TWINT (gotowość po zakończeniu operacji) i globalny bit I = 1
    ww1.microchip.com
    . W ISR TWI_vect czytamy/ustawiamy dane w TWI (TWDR, TWSR). Pozwala to na programową obsługę przesyłu TWI na wyższym poziomie.

    SPI: Interfejs szeregowy SPI może wyzwalać przerwanie po zakończeniu przesłania jednego bajtu. Po ustawieniu bitu SPIE w rejestrze SPCR i gdy przyjdzie flaga SPIF (Transfer Complete), generowany jest wektor SPI_STC_vect. W ISR można odczytać wysłaną wartość z SPDR lub przygotować kolejny bajt. Umożliwia to asynchroniczne przesyłanie danych SPI.

    Inne: ATmega328P posiada też przerwania wewnętrzne jak ADC Noise Reduction Sleep Wakeup, USART0 Receive Complete, SPI Serial Transfer Complete, TWI Address Match, EEPROM Ready, Analog Comparator, Watchdog Timer itp., które pozwalają na reagowanie na różne zdarzenia peryferiów mikrokontrolera. Wykorzystuje się je np. do obudzenia z trybów uśpienia lub obsługi sprzętowych funkcji.

Każdy z powyższych modułów ma dedykowane bity Enable przerwania oraz flagi zdarzeń opisane w dokumentacji. W praktycznym zastosowaniu należy ustawić odpowiednie bity w rejestrach sterujących (np. ADCSRA dla ADC, UCSR0B dla USART, EIMSK/EICRA dla INT0/1, PCICR/PCMSKx dla PCINT, TWCR dla TWI, SPCR dla SPI) i włączyć przerwania globalnie. Powoduje to wywołanie odpowiedniego procedury obsługi (ISR), w której realizujemy pożądaną reakcję na zdarzenie (odczyt rejestrów, przetwarzanie danych, ustawienie pinów).

Podsumowanie: Timery w ATmega328P są elastyczne i umożliwiają generowanie przerwań w różnych trybach (przepełnienie, compare match, capture), przy użyciu prostych rejestrów TCCRn, OCRn, TIMSKn i TIFRn. Wystarczy ustawić preskaler, wartość porównania (lub początkowy TCNT), włączyć lokalne przerwanie w TIMSKn i globalne (sei()), a mikroprocesor zacznie wywoływać ISR w zadanych momentach. Pozostałe wewnętrzne przerwania (ADC, UART, INT0/1, PCINT, TWI, SPI) działają podobnie – mają swoje rejestry sterujące przerwaniami i własne wektory. Dzięki temu ATmega328P może obsługiwać wiele zadań w tle (mierzenie czasu, komunikacja, pomiary ADC itp.) w sposób efektywny i precyzyjny.

Źródła: Parametry timerów i opis bitów przerwań pochodzą z dokumentacji ATmega328P. Przykładowe kody oparto na materiałach edukacyjnych i praktycznych poradnikach (AVR tutorial). Omówienie pozostałych przerwań bazuje na opisie funkcji ADC, USART, TWI, SPI i przerwań zewnętrznych w dokumentacji mikrokontrolera.
