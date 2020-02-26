-----Read-me-----

Program: time_based.c
----------------
Time_based er vores tidsbaserede løsning

__Guide: Simuler med standard indstillinger__
1. Compile time_based.c
2. Kør time_based.exe
3. Indtast 1 som input for at vise simuleringens grafik
4. Indtast et starttidspunkt (målt i sekunder efter midnat)
5. Vælg en simulerings timescale. 1 er tilsvarende naturlig hastighed og 2 er lig dobbelt hastighed osv. i forhold til den virkelige verden.


P.S: Der kan simuleres flere dage ad gangen ved at ændre værdien i while løkken i funktionen sim_time_based():

sim_state->days_simulated < 1
----------------------------^