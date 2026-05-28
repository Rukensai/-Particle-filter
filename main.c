#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_PARTICLES 200  // Количество частиц для симуляции
#define STEPS 50 // Кол-во шагов

// Здесь как мы храним частицы
typedef struct{
    double x;  // Её позиция(по линии)
    double w;  // И вес(вероятность)
} Particle;

// Нормальное распределенние чисел (Box-Muller)
double randn(double mean, double miss){
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    
    // Чтобы не было логарифма от нуля(-inf)
    if (u1 <= 1e-7) u1 = 1e-7; 
    
    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2); // Просто чутка адаптированная формула с вики, просто берём только одну z, т.к. нам только одну величину генерить;)
    return mean + z0 * miss; // Мы распределние добавляем к примерной позиции
}

void init_particles(Particle* particles, int n){
    double initial_mean = 0.0; // точка отсчёта(потом меняем)
    double initial_miss = 2.0; // Согласно p(x0) = N(0, 2^2) по подфке
    double initial_weight = 1.0 / n; // 100% на все частицы делим, т.е. каждый равен в проценте своём
    
    for (int i = 0; i < n; i++){
        // Позиции по норм распределению
        particles[i].x = randn(initial_mean, initial_miss);
        // Ставим начальный вес(по базе все равны, как при Союзе)
        particles[i].w = initial_weight;
    }
}

// movement - эт насколько я ожидаю переместиться
// miss   - погрешность шага
void predict(Particle* particles, int n, double movement, double miss){
    for (int i = 0; i < n; i++) {
        // Для каждой отдельной частицы генерим рандомную ошибку в пределах погрешности
        double noise = randn(0.0, miss);
        
        // Новая позиция = старая позиция + шаг + ошибка
        particles[i].x += movement + noise;
    }
}
// Обновление весов
// z - измерение датчика
// R - наша погрешность
void update_weights(Particle* particles, int n, double z, double R){
    for (int i = 0; i < n; i++){
        double diff = particles[i].x - z;
        particles[i].w = (1.0 / sqrt(2.0 * M_PI * R)) * exp(-(diff * diff) / (2.0 * R)); // Формула функции Гаусса
    }
}
// Теперь уже нормализуем весы после апдейта
void normalize_weights(Particle* particles, int n){
    double sum = 0.0;
    // Сумма весов
    for (int i = 0; i < n; i++){
        sum += particles[i].w;
    }
    
    // Делим каждый вес на эту сумму(ну не ноль, канеш)
    if (sum > 0){
        for (int i = 0; i < n; i++){
            particles[i].w /= sum;
        }
    } else{
        // Если все веса стали 0 (фильтр вмер), сбрасываем всех к равному весу
        for (int i = 0; i < n; i++){
            particles[i].w = 1.0 / n;
        }
    }
}
// Перевыборка
void resample(Particle* particles, int n){
    Particle new_particles[n];
    double weights[n];
    
    for(int i = 0; i < n; i++) weights[i] = particles[i].w;
    
    // Алгоритм выбора частиц пропорционально их весам
    double total_weight = 0;
    double cumulative_sum[n];
    for(int i = 0; i < n; i++){
        total_weight += weights[i];
        cumulative_sum[i] = total_weight;
    }
    
    for(int i = 0; i < n; i++){
        double r = (double)rand() / RAND_MAX * total_weight;
        // Находим частицу, чей кумулятивный вес больше случайного числа r
        for(int j = 0; j < n; j++){
            if (cumulative_sum[j] >= r){
                new_particles[i] = particles[j];
                break;
            }
        }
    }
    
    // Копируем обратно в исходный массив
    for(int i = 0; i < n; i++) {
        particles[i] = new_particles[i];
        particles[i].w = 1.0 / n; // После перевыборки сбрасываем веса
    }
}
// уже оцениваем хде мы
double estimate_position(Particle* particles, int n){
    double estimate = 0.0;
    for (int i = 0; i < n; i++){
        estimate += particles[i].x * particles[i].w; // берём среднее
    }
    return estimate;
}

int main() 
{
    srand((unsigned int)time(NULL));
    Particle particles[NUM_PARTICLES];
    
    // Начальное состояние
    init_particles(particles, NUM_PARTICLES);
    
    double true_x = 0.0;
    FILE* fp = fopen("output.csv", "w");
    fprintf(fp, "step,true_x,measured_x,estimated_x\n");

    for (int t = 0; t < STEPS; t++){
        // Истинная траектория
        true_x += 1.0; 
        double measurement = true_x + randn(0.0, 1.0); // Датчик врет на 1.0
        
        // Шаги фильтра
        predict(particles, NUM_PARTICLES, 1.0, 0.5);
        update_weights(particles, NUM_PARTICLES, measurement, 1.0);
        normalize_weights(particles, NUM_PARTICLES);
        resample(particles, NUM_PARTICLES);
        
        // Оценка
        double estimate = estimate_position(particles, NUM_PARTICLES);
        
        // Запись уже в сам файл
        fprintf(fp, "%d,%.2f,%.2f,%.2f\n", t, true_x, measurement, estimate);
        printf("Step %d: True=%.2f, Est=%.2f\n", t, true_x, estimate);
    }
    
    fclose(fp);
    return 0;
}
