#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_PARTICLES 200  // Количество частиц для симуляции

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
    return mean + z0 * miss; // Мы распределние добавляем к примерной позиции(пока чисто к начальной точке 0.0)
}

void init_particles(Particle* particles, int n){
    double initial_mean = 0.0; // точка отсчёта
    double initial_miss = 2.0; // Согласно p(x0) = N(0, 2^2) по подфке(в общем на сколько раскидаем частицы для поиска шага, эт значение из пдфки)
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
    for (int i = 0; i < n; i++){
        // Для каждой отдельной частицы генерим рандомную ошибку в пределах погрешности
        double noise = randn(0.0, miss);
        
        // Новая позиция = старая позиция + шаг + ошибка
        particles[i].x += movement + noise;
    }
}

int main() 
{
    // Запускаем генератор случайных чисел по времени провед. в симуляции
    srand((unsigned int)time(NULL));

    Particle particles[NUM_PARTICLES]; // Инициализируем массив с мусором под частицы
    
    init_particles(particles, NUM_PARTICLES);
    // Тестовый вывод, чтобы убедиться, что частицы генерятся
    for (int i = 0; i < 5; i++){
        printf("Particle %d x = %7.4f, w = %7.4f\n", i, particles[i].x, particles[i].w);
    }

    double step = 1.0;          // Для проверки один шаг сделаем
    double process_noise = 0.5; // Зададим погрешность измерения
    
    predict(particles, NUM_PARTICLES, step, process_noise);

    
    // Тестим-с наши предикты
    printf("Test first 5 predicts:\n");
    for (int i = 0; i < 5; i++){
        printf("Particle %d x = %7.4f, w = %7.4f\n", i, particles[i].x, particles[i].w);
    }

    return 0;
}
