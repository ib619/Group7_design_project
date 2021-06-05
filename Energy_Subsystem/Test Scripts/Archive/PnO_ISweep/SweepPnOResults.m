clear; close all;

% import = readmatrix('SWEEPNO2.CSV');

%%
import1 = load('sweeptest1.mat');
import2 = load('sweeptest2.mat');

%%
I_out_ref_1 = import1.import(:,3);
I_out_1 = import1.import(:,4);
V_out_1 = import1.import(:,2);

I_out_ref_2 = import2.import(:,3);
I_out_2 = import2.import(:,4);
V_out_2 = import2.import(:,2);

%%

figure(1);
plot(V_out_1, I_out_1, 'rx'); hold on;
plot(V_out_2, I_out_2, 'bx');
ylabel('I_{out}');
xlabel('V_{out}');



