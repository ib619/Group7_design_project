clear; close all;

PWM = [0
15
30
45
60
75
90
105
120
135
150
165
180
195
210
225
240
255];

Pwr = [2.29112
2.3204
2.33948
2.35848
2.3774
2.4025
2.45135
2.44133
2.47139
2.48
2.51
2.52
2.53
2.54
2.55843
2.56841
2.58338
2.58837];

%%

figure(1);
plot(PWM, Pwr, 'x','Linewidth', 2, 'MarkerSize', 8)
xlabel('PWM (%)');
ylabel('Power Drawn (W)');
title('PWM against Power Drawn');
set(gca,'FontSize',12)
set(gca,'LineWidth',2)
set(gca, 'FontName', 'Arial')
exportgraphics(gcf, strcat('drivepwr.jpg'), 'Resolution',300) 
