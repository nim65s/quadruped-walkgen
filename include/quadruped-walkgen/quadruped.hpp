#ifndef __quadruped_walkgen_quadruped_hpp__
#define __quadruped_walkgen_quadruped_hpp__
#include <stdexcept>

#include "crocoddyl/core/fwd.hpp"
#include "crocoddyl/core/action-base.hpp"
#include "crocoddyl/core/states/euclidean.hpp"
#include "crocoddyl/multibody/friction-cone.hpp"
#include "crocoddyl/core/activations/quadratic-barrier.hpp"

namespace quadruped_walkgen {
template <typename _Scalar>
class ActionModelQuadrupedTpl : public crocoddyl::ActionModelAbstractTpl<_Scalar> {
 public:
  typedef _Scalar Scalar;
  typedef crocoddyl::ActionDataAbstractTpl<Scalar> ActionDataAbstract;
  typedef crocoddyl::ActionModelAbstractTpl<Scalar> Base;
  typedef crocoddyl::MathBaseTpl<Scalar> MathBase;
  

  
  ActionModelQuadrupedTpl();
  ~ActionModelQuadrupedTpl();

  virtual void calc(const boost::shared_ptr<ActionDataAbstract>& data,
                    const Eigen::Ref<const typename MathBase::VectorXs>& x,
                    const Eigen::Ref<const typename MathBase::VectorXs>& u);
  virtual void calcDiff(const boost::shared_ptr<ActionDataAbstract>& data,
                        const Eigen::Ref<const typename MathBase::VectorXs>& x,
                        const Eigen::Ref<const typename MathBase::VectorXs>& u);
  virtual boost::shared_ptr<ActionDataAbstract> createData();

  // Get and Set weights vectors : state & force
  const typename Eigen::Matrix<Scalar, 12, 1>& get_force_weights() const;
  void set_force_weights(const typename MathBase::VectorXs& weights);

  const typename Eigen::Matrix<Scalar, 12, 1>& get_state_weights() const;
  void set_state_weights(const typename MathBase::VectorXs& weights);

  // Update the model dependinf if the foot in contact with the ground 
  // or the new lever arms
  void update_model(const Eigen::Ref<const typename MathBase::MatrixXs>& l_feet  ,
                    const Eigen::Ref<const typename MathBase::VectorXs>& xref,
                    const Eigen::Ref<const typename MathBase::MatrixXs>& S ) ;

  // Get Skew matrix, usefull for the computation of B (cross product)
  const typename Eigen::Matrix<Scalar, 3, 3 >& get_skew(const typename Eigen::Matrix<Scalar, 3, 1 >& vec) const;

  // Get A & B matrix
  const typename Eigen::Matrix<Scalar, 12, 12 >& get_A() const;
  const typename Eigen::Matrix<Scalar, 12, 12 >& get_B() const;
  

 protected:
  using Base::has_control_limits_;  //!< Indicates whether any of the control limits
  using Base::nr_;                  //!< Dimension of the cost residual
  using Base::nu_;                  //!< Control dimension
  using Base::state_;               //!< Model of the state
  using Base::u_lb_;                //!< Lower control limits
  using Base::u_ub_;                //!< Upper control limits
  using Base::unone_;               //!< Neutral state
 
 
 private:
  typename MathBase::Vector2s cost_weights_;
  typename Eigen::Matrix<Scalar, 12, 1> force_weights_;
  typename Eigen::Matrix<Scalar, 12, 1> state_weights_;
  Scalar dt_;
  Scalar mass ; 
  Scalar mu ;
  typename Eigen::Matrix<Scalar, 12, 1> g;
  typename Eigen::Matrix<Scalar, 12, 1> xref_;
  typename Eigen::Matrix<Scalar, 3 , 3 > gI;
  typename Eigen::Matrix<Scalar, 3, 3> R ; 
  typename Eigen::Matrix<Scalar, 12, 12 > A;
  typename Eigen::Matrix<Scalar, 12, 12 > B;
  typename Eigen::Matrix<Scalar, 3, 4> lever_arms;
  typename MathBase::Vector3s lever_tmp;
  typename MathBase::Matrix3s R_tmp ;

   
  typename Eigen::Matrix<Scalar, 20, 1 > lb ; 
  typename Eigen::Matrix<Scalar, 20, 1 > ub ; 
  typename Eigen::Matrix<Scalar, 20, 12 > Fa ; 
  typename Eigen::Matrix<Scalar, 3, 1 > nsurf ; 
  
  crocoddyl::ActivationModelQuadraticBarrierTpl<Scalar> activation ; 
  crocoddyl::FrictionConeTpl<Scalar> cone ;
  crocoddyl::ActivationDataQuadraticBarrierTpl<Scalar> dataCost ; // Problem 
  

   
  
};

template <typename _Scalar>
struct ActionDataQuadrupedTpl : public crocoddyl::ActionDataAbstractTpl<_Scalar> {
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  typedef _Scalar Scalar;
  typedef crocoddyl::MathBaseTpl<Scalar> MathBase;
  typedef crocoddyl::ActionDataAbstractTpl<Scalar> Base;
  using Base::cost;
  using Base::Fu;
  using Base::Fx;
  using Base::Lu;
  using Base::Luu;
  using Base::Lx;
  using Base::Lxu;
  using Base::Lxx;
  using Base::r;
  using Base::xnext;

  template <template <typename Scalar> class Model>
  explicit ActionDataQuadrupedTpl(Model<Scalar>* const model) : crocoddyl::ActionDataAbstractTpl<Scalar>(model) {}
};

  

/* --- Details -------------------------------------------------------------- */
/* --- Details -------------------------------------------------------------- */
/* --- Details -------------------------------------------------------------- */




typedef ActionModelQuadrupedTpl<double> ActionModelQuadruped;
typedef ActionDataQuadrupedTpl<double> ActionDataQuadruped;
typedef crocoddyl::ActionModelAbstractTpl<double> ActionModelAbstract ; 
typedef crocoddyl::ActionDataAbstractTpl<double> ActionDataAbstract ; 
typedef crocoddyl::StateAbstractTpl<double> StateAbstract;

typedef crocoddyl::ActionModelUnicycleTpl<double> ActionModelUnicycle;
typedef crocoddyl::ActionDataUnicycleTpl<double> ActionDataUnicycle;




}

#include "quadruped.hxx"


#endif