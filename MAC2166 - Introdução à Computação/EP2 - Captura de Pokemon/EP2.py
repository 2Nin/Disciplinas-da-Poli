"""
  AO PREENCHER ESSE CABEÇALHO COM O MEU NOME E O MEU NÚMERO USP, 
  DECLARO QUE SOU O ÚNICO AUTOR E RESPONSÁVEL POR ESSE PROGRAMA. 
  TODAS AS PARTES ORIGINAIS DESSE EXERCÍCIO PROGRAMA (EP) FORAM 
  DESENVOLVIDAS E IMPLEMENTADAS POR MIM SEGUINDO AS INSTRUÇÕES
  DESSE EP E QUE PORTANTO NÃO CONSTITUEM DESONESTIDADE ACADÊMICA
  OU PLÁGIO.  
  DECLARO TAMBÉM QUE SOU RESPONSÁVEL POR TODAS AS CÓPIAS
  DESSE PROGRAMA E QUE EU NÃO DISTRIBUI OU FACILITEI A
  SUA DISTRIBUIÇÃO. ESTOU CIENTE QUE OS CASOS DE PLÁGIO E
  DESONESTIDADE ACADÊMICA SERÃO TRATADOS SEGUNDO OS CRITÉRIOS
  DIVULGADOS NA PÁGINA DA DISCIPLINA.
  ENTENDO QUE EPS SEM ASSINATURA NÃO SERÃO CORRIGIDOS E,
  AINDA ASSIM, PODERÃO SER PUNIDOS POR DESONESTIDADE ACADÊMICA.

  Nome : Antônio Augusto Carnevalli
  NUSP : 13682909
  Professor : Alair Pereira do Lago
  Turma: 11 - Engenharia Mecatrônica

  Referências: Com exceção das rotinas fornecidas no enunciado
  e em sala de aula, caso você tenha utilizado alguma refência,
  liste-as abaixo para que o seu programa não seja considerado
  plágio ou irregular.
  
  -Aula do IME/USP sobre funções recursivas:
  http://www.vision.ime.usp.br/~pmiranda/mac2166_1s17/aulas/P3/aulas_P3.html


"""
GRAVIDADE = 9.81
PI = 3.14159265358979323846

def fatorial(x): #Calculadora de fatoriais, já que não é possível importar a biblioteca math.
    i = x
    fat = x
    while i > 1:
        i -= 1
        fat = fat*i
    return fat

def radianos(theta): #Convertor de graus em radianos.
    thetaRad = (theta * PI)/180

    return thetaRad

def seno(theta):
    '''
    Esta função aproxima o valor da função seno para o ângulo theta
    usando a série de Taylor até que o módulo do próximo termo da
    série calculada seja menor 1e-10.
    Entrada: O ângulo theta que deve ser informado em graus.
    Saída: A aproximação do seno do ângulo theta.
    '''

    thetaRad = radianos(theta)
    senotheta = thetaRad

    i = 1
    while abs((((-1)**i)/(fatorial(2*i+1)))*(thetaRad**(2*i+1))) > (10 ** (-10)):
        senotheta += ((-1) ** i)/fatorial(2 * i + 1) * (thetaRad ** (2 * i + 1))
        i += 1
    return senotheta


def cosseno(theta):
    '''
    Esta função aproxima o valor da função cosseno para o ângulo theta
    usando a série de Taylor até que o módulo do próximo termo da
    série calculada seja menor 1e-10.
    Entrada: O ângulo theta que deve ser informado em graus.
    Saída: A aproximação do cosseno do ângulo theta.
    '''
    thetaRad = radianos(theta)
    cossenotheta = seno(90 - theta) #Utilizando conceitos de trigonometria para calcular o cosseno a partir do seno, para não repetir fórmulas.

    return cossenotheta


def raizQuadrada(x):
    '''
    Esta função aproxima o valor da raiz quadrada de x, através da
    fórmula de recorrência r_0 = x e r_{n+1} = 1/2 (r_n+ x/r_n)
    enquanto o módulo da diferença entre os dois últimos valores
    calculados for maior que 1e-10.
    Entrada: O valor de x
    Saída: A aproximação da raiz quadrada de x.
    '''
    rn = x
    rc = (rn + (x/rn))/2
    dif = rn - rc
    while abs(dif) > 10**-10:
        rc = (rn + (x/rn)) * (1/2)
        dif = rn - rc
        rn = rc
    return rc

def atualizaPosicao(x, y, vx, vy, delta_t):
    '''
    Esta função calcula as atualizações das posições de x e y usando
    as velocidades escalares respectivamente dadas por vx e vy.
    Entrada: As posições x e y dadas em metros, as velocidades vx e
    vy em metros por segundo e o intervalo de tempo em segundos.
    Saída: Dois valores: o valor atualizado de x e o valor atualizado de y.
    '''
    x = x + (vx * delta_t)
    y = y + (vy * delta_t - (GRAVIDADE * (delta_t ** 2))/2)
    return x, y


def atualizaVelocidade(vx, vy, delta_t):
    '''
    Esta função calcula e atualiza as velocidades vx e vy para o
    próximo intervalo de tempo.
    Entrada: As velocidades vx e vy em metros por segundo e o
    intervalo de tempo em segundos.
    Saída: Dois valores: o valor atualizado de vx e o valor atualizado de vy.
    '''
    vx = vx #Constante (não há aceleração em x)
    vy = vy - (GRAVIDADE * delta_t)
    return vx, vy


def distanciaPontos(x1, y1, x2, y2):
    '''
    Esta função calcula a distância entre dois pontos (x1, y1) e (x2, y2).
    Entrada: As coordenadas dos pontos do plano (x1, y1) e (x2, y2).
    Saída: A distância entre (x1, y1) e (x2, y2).
    '''
    distancia = raizQuadrada((x2 - x1)**2 + (y2 - y1)**2) #distância dos pontos utilizando uma relação pitagórica vista em geometria analítica
    return distancia



def simulaLancamento(xpokebola, ypokebola, rb, vlancamento, angulolancamento, xp, yp, rp, delta_t):
    '''

    Esta função simula o lançamento da bola até que ela capture o
    pokemon, ou atinja o chão.
    Entrada: Posição inicial da pokebola (xpokebola e ypokebola), em metros;
    Posição do pokemon (xpokemon e ypokemon), em metros;
    Velocidade escalar, em metros por segundo;
    e ângulo de lançamento, em graus;
    Os raios rb e rp, em metros;
    a granularidade de tempo delta_t usada na simulação, em segundos.
    Saída: Um booleano (True se o lançamento teve sucesso e acertou o 
    pokémon, ou False caso contrário), a menor distância do pokémon à
    pokébola e as coordenadas x e y da pokébola nesta posição mais próxima.
    '''
    
    menordistancia = distanciaPontos(xpokebola, ypokebola, xp, yp)
    menorx, menory = xpokebola, ypokebola
    captura = False
    vx = (vlancamento * cosseno(angulolancamento))
    vy = (vlancamento * seno(angulolancamento))
    xpokebola, ypokebola = atualizaPosicao(xpokebola, ypokebola, vx, vy, delta_t)
    vx, vy = atualizaVelocidade(vx, vy, delta_t)
    while ypokebola > rb:
        if distanciaPontos(xpokebola, ypokebola, xp, yp) <= abs(rb + rp):
            captura = True
            menordistancia = 0
            break
        xpokebola, ypokebola = atualizaPosicao(xpokebola, ypokebola, vx, vy, delta_t)
        vx, vy = atualizaVelocidade(vx, vy, delta_t)
        if menordistancia > distanciaPontos(xpokebola, ypokebola, xp, yp) - abs(rp + rb): #Atualiza a menor distancia
            menordistancia = distanciaPontos(xpokebola, ypokebola, xp, yp) - abs(rp + rb)
            menorx = xpokebola
            menory = ypokebola

    return captura, menordistancia, menorx, menory


def main():
    xp = float(input("\nDigite a coordenada x do pokemon: "))
    yp = float(input("\nDigite a coordenada y do pokemon: "))
    rp = float(input("\nDigite o raio do pokemon (> 0) em metros:"))
    rb = float(input("\nDigite o raio da pokebola (> 0) em metros:"))
    delta_t = float(input("\nDigite a granularidade de tempo da simulacao em segundos:"))
    tentativas = 1
    while (tentativas <= 3):
        print("\nTentativa", tentativas)
        xpokebola = float(input("\nDigite a coordenada x do treinador:"))
        ypokebola = float(input("\nDigite a coordenada y do treinador:"))
        vlancamento = float(input("\nDigite a velocidade de lancamento em m/s:"))
        angulolancamento = float(input("\nDigite o angulo de lancamento em graus:"))
        captura, menordistancia, menorx, menory = simulaLancamento(xpokebola, ypokebola, rb, vlancamento, angulolancamento, xp, yp, rp, delta_t)
        if captura == True:
            print("\nA pokebola captura o pokemon.")
            break
        elif captura == False:
            print("\nA pokebola nao captura o pokemon por %.4f metros," %menordistancia, "ao passar em %.4f" %menorx, ",%.4f." %menory)
        tentativas += 1
        

# Não altere o código abaixo:
if __name__ == "__main__":
    main()