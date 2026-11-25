#include <iostream>
#include <string>

namespace maths {

namespace core {

using term_t = float;
using exp_t = unsigned int;

inline term_t power(term_t base, exp_t exp) {
  term_t result = 1.0f;
  for (exp_t i = 0; i < exp; ++i)
    result *= base;
  return result;
}

template <int N> class Expression {
public:
  virtual ~Expression() = default;
  virtual term_t solve(const term_t vals[N]) const = 0;
  virtual void differentiate(Expression<N> &out, const exp_t mask[N]) const = 0;
  virtual std::string toString() const = 0;
};

}

namespace poly {

using core::exp_t;
using core::Expression;
using core::term_t;

template <int N> class Monomial : public Expression<N> {
protected:
  term_t coeff;
  exp_t exps[N];

public:
  Monomial() { nullify(); }
  explicit Monomial(term_t coeff) : coeff(coeff) {
    for (int i = 0; i < N; ++i) exps[i] = 0;
  }
  Monomial(term_t coeff, const exp_t exps[N]) : coeff(coeff) { setExponents(exps); }

  term_t getCoeff() const { return coeff; }
  void getExponents(exp_t out[N]) const {
    for (int i = 0; i < N; ++i) out[i] = exps[i];
  }
  void nullify() {
    coeff = 0.0f;
    for (int i = 0; i < N; ++i) exps[i] = 0;
  }
  void setExponents(const exp_t e[N]) {
    for (int i = 0; i < N; ++i) exps[i] = e[i];
  }

  void differentiate(Expression<N> &out, const exp_t mask[N]) const override {
    differentiate(dynamic_cast<Monomial<N> &>(out), mask);
  }
  void differentiate(Monomial<N> &deriv, const exp_t mask[N]) const {
    deriv.coeff = coeff;
    deriv.setExponents(exps);
    bool derived = false;
    for (int i = 0; i < N; ++i) {
      if (mask[i] == 0) continue;
      if (exps[i] == 0) { deriv.nullify(); return; }
      deriv.coeff *= exps[i];
      deriv.exps[i] -= 1;
      derived = true;
    }
    if (!derived) deriv.nullify();
  }

  term_t solve(const term_t vals[N]) const override {
    term_t result = coeff;
    for (int i = 0; i < N; ++i) result *= core::power(vals[i], exps[i]);
    return result;
  }

  std::string toString() const override {
    std::string result = std::to_string(coeff);
    for (int i = 0; i < N; ++i)
      if (exps[i] != 0)
        result += "x_" + std::to_string(i + 1) + "^" + std::to_string(exps[i]);
    return result;
  }
};

template <int N, int M> class Polynomial : public Expression<N> {
protected:
  Monomial<N> *terms[M];

  Polynomial() = default;

public:
  explicit Polynomial(Monomial<N> *const t[M]) {
    for (int i = 0; i < M; ++i) terms[i] = t[i];
  }

  term_t solve(const term_t vals[N]) const override {
    term_t result = 0.0f;
    for (int i = 0; i < M; ++i) result += terms[i]->solve(vals);
    return result;
  }

  void differentiate(Expression<N> &out, const exp_t mask[N]) const override {
    differentiate(dynamic_cast<Polynomial<N, M> &>(out), mask);
  }
  void differentiate(Polynomial<N, M> &deriv, const exp_t mask[N]) const {
    for (int i = 0; i < M; ++i) terms[i]->differentiate(*deriv.terms[i], mask);
  }

  std::string toString() const override {
    std::string result;
    for (int i = 0; i < M; ++i) {
      if (i > 0 && terms[i]->getCoeff() >= 0) result += "+";
      result += terms[i]->toString();
    }
    return result;
  }
};

namespace uni {

constexpr int DIM = 1;

class Monomial1D : public Monomial<DIM> {
public:
  using Monomial<DIM>::Monomial;
};

template <int M> class Polynomial1D : public Polynomial<DIM, M> {
public:
  explicit Polynomial1D(Monomial1D *const mons[M]) {
    for (int i = 0; i < M; ++i)
      this->terms[i] = mons[i];
  }
};

} // namespace uni
} // namespace poly
} // namespace maths

int main() {
  using namespace maths::poly::uni; 
  using maths::core::exp_t;
  using maths::core::term_t;

  term_t vals[1] = {3.0f};
  const exp_t exps[1] = {2u};
  const exp_t mask[1] = {1u};

  Monomial1D m1(12.4f, exps), m2(-18.f, exps), dm1;
  Monomial1D *ms[2] = {&m1, &m2};
  Monomial1D d1, d2, e1, e2;
  Monomial1D *dms[2] = {&d1, &d2}, *ems[2] = {&e1, &e2};

  m1.differentiate(dm1, mask);
  std::cout << "m1'      = " << dm1.toString() << '\n';

  Polynomial1D<2> p(ms), dp(dms), dp2(ems);
  p.differentiate(dp, mask);
  std::cout << "p        = " << p.toString() << '\n';
  std::cout << "p(3)     = " << p.solve(vals) << '\n';
  std::cout << "p'       = " << dp.toString() << '\n';

  maths::core::Expression<1> *e = &p;
  e->differentiate(dp2, mask);
  std::cout << "p' (via Expression) = " << dp2.toString() << '\n';
  return 0;
}