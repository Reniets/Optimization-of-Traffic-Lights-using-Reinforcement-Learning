-----Read-me-----

Program: time_based.c
----------------
Time_based er vores tidsbaserede l�sning

__Guide: Simuler med standard indstillinger__
1. Compile time_based.c
2. K�r time_based.exe
3. Indtast 1 som input for at vise simuleringens grafik
4. Indtast et starttidspunkt (m�lt i sekunder efter midnat)
5. V�lg en simulerings timescale. 1 er tilsvarende naturlig hastighed og 2 er lig dobbelt hastighed osv. i forhold til den virkelige verden.


P.S: Der kan simuleres flere dage ad gangen ved at �ndre v�rdien i while l�kken i funktionen sim_time_based():

sim_state->days_simulated < 1
----------------------------^