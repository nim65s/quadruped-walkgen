#ifndef __quadruped_walkgen_quadruped_nl_hxx__
#define __quadruped_walkgen_quadruped_nl_hxx__

#include "crocoddyl/core/utils/exception.hpp"


namespace quadruped_walkgen  {
template <typename Scalar>
ActionModelQuadrupedNonLinearTpl<Scalar>::ActionModelQuadrupedNonLinearTpl()
    : crocoddyl::ActionModelAbstractTpl<Scalar>(boost::make_shared<crocoddyl::StateVectorTpl<Scalar> >(12), 12, 24)
  {
  // Model parameters
  mu = Scalar(1) ; 
  dt_ = Scalar(0.02) ; 
  mass = Scalar(2.50000279) ; 
  min_fz_in_contact = Scalar(0.0) ; 

  // Matrix model initialization
  g.setZero() ;
  g[8] = Scalar(-9.81)*dt_ ;
  gI.setZero() ; 
  gI.diagonal() << Scalar(0.00578574) , Scalar(0.01938108) , Scalar(0.02476124) ; 
  A.setIdentity() ; 
  A.topRightCorner(6,6) << Eigen::Matrix<Scalar, 6, 6>::Identity()*dt_ ; 
  B.setZero() ; 
  lever_arms.setZero() ; 
  R.setZero() ; 
 

  // Weight vectors initialization
  force_weights_.setConstant(0.2);
  state_weights_ << Scalar(1.)  , Scalar(1.) , Scalar(150.) , Scalar(35.),
                    Scalar(30.) , Scalar(8.) , Scalar(20.)  , Scalar(20.) , 
                    Scalar(15.) , Scalar(4.) , Scalar(4.)   , Scalar(8.)  ; 
  friction_weight_ = Scalar(10) ;
  
  // UpperBound vector 
  ub.setZero() ; 

  // Temporary vector used 
  Fa_x_u.setZero() ; 
  rub_.setZero() ; 
  rub_max_.setZero() ;
  Arr.setZero() ; 
  r.setZero() ;
  lever_tmp.setZero() ;
  R_tmp.setZero() ; 
  gait.setZero() ; 
  base_vector_x << Scalar(1.) , Scalar(0.) , Scalar(0.) ;
  base_vector_y << Scalar(0.) , Scalar(1.) , Scalar(0.) ;
  base_vector_z << Scalar(0.) , Scalar(0.) , Scalar(1.) ; 
  forces_3d.setZero() ; 

  // Used for shoulder height weight
  pshoulder_0 <<  Scalar(0.1946) ,   Scalar(0.1946) ,   Scalar(-0.1946),  Scalar(-0.1946) , 
                  Scalar(0.15005) ,  Scalar(-0.15005)  , Scalar(0.15005)  ,  Scalar(-0.15005) ; 
  shoulder_height_limit = Scalar(0.24) ; 
  shoulder_height_weight = Scalar(100.) ;
  sh_ub_max_.setZero() ; 
}


template <typename Scalar>
ActionModelQuadrupedNonLinearTpl<Scalar>::~ActionModelQuadrupedNonLinearTpl() {}


template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::calc(const boost::shared_ptr<crocoddyl::ActionDataAbstractTpl<Scalar> >& data,
                                          const Eigen::Ref<const typename MathBase::VectorXs>& x,
                                          const Eigen::Ref<const typename MathBase::VectorXs>& u) {
  if (static_cast<std::size_t>(x.size()) != state_->get_nx()) {
    throw_pretty("Invalid argument: "
                 << "x has wrong dimension (it should be " + std::to_string(state_->get_nx()) + ")");
  }
  if (static_cast<std::size_t>(u.size()) != nu_) {
    throw_pretty("Invalid argument: "
                 << "u has wrong dimension (it should be " + std::to_string(nu_) + ")");
  }

  ActionDataQuadrupedNonLinearTpl<Scalar>* d = static_cast<ActionDataQuadrupedNonLinearTpl<Scalar>*>(data.get());

  //  Update B : 
  for (int i=0; i<4; i=i+1){
    if (gait(i,0) != 0) {
      lever_tmp = lever_arms.block(0,i,3,1) - x.block(0,0,3,1) ;
      R_tmp << 0.0, -lever_tmp[2], lever_tmp[1],
      lever_tmp[2], 0.0, -lever_tmp[0], -lever_tmp[1], lever_tmp[0], 0.0 ; 
      B.block(9 , 3*i  , 3,3) << dt_ * R* R_tmp; 
    }    
  } ; 

 
  // Discrete dynamic : A*x + B*u + g
  d->xnext << A.diagonal().cwiseProduct(x) + g ; 
  d->xnext.template head<6>() = d->xnext.template head<6>() + A.topRightCorner(6,6).diagonal().cwiseProduct(x.tail(6)) ;
  d->xnext.template tail<6>() = d->xnext.template tail<6>() + B.block(6,0,6,12)*u;
  
  // Residual cost on the state and force norm
  d->r.template head<12>() =  state_weights_.cwiseProduct(x - xref_);
  d->r.template tail<12>() =  force_weights_.cwiseProduct(u);

  // Friction cone + shoulder height
  for (int i=0; i<4; i=i+1){
     Fa_x_u.segment(5*i,5) << u(3*i) - mu*u(3*i+2) , -u(3*i) - mu*u(3*i+2),
                              u(3*i+1) - mu*u(3*i+2) , -u(3*i+1) - mu*u(3*i+2),
                              -u(3*i+2) ;                       
  }
  rub_max_ = (Fa_x_u - ub).cwiseMax(Scalar(0.))   ;

  // Shoulder height weight
  sh_ub_max_ << x(2) + pshoulder_0(1,0)*x(3) - pshoulder_0(0,0)*x(4) - shoulder_height_limit ,
                x(2) + pshoulder_0(1,1)*x(3) - pshoulder_0(0,1)*x(4) - shoulder_height_limit ,
                x(2) + pshoulder_0(1,2)*x(3) - pshoulder_0(0,2)*x(4) - shoulder_height_limit , 
                x(2) + pshoulder_0(1,3)*x(3) - pshoulder_0(0,3)*x(4) - shoulder_height_limit ; 
  sh_ub_max_ = sh_ub_max_.cwiseMax(Scalar(0.)) ;    
  
  // Cost computation 
  d->cost = 0.5 * d->r.transpose() * d->r     + friction_weight_ * Scalar(0.5) * rub_max_.squaredNorm() + shoulder_height_weight * Scalar(0.5) * sh_ub_max_.squaredNorm() ;
}


template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::calcDiff(const boost::shared_ptr<crocoddyl::ActionDataAbstractTpl<Scalar> >& data,
                                              const Eigen::Ref<const typename MathBase::VectorXs>& x,
                                              const Eigen::Ref<const typename MathBase::VectorXs>& u) {
  if (static_cast<std::size_t>(x.size()) != state_->get_nx()) {
    throw_pretty("Invalid argument: "
                 << "x has wrong dimension (it should be " + std::to_string(state_->get_nx()) + ")");
  }
  if (static_cast<std::size_t>(u.size()) != nu_) {
    throw_pretty("Invalid argument: "
                 << "u has wrong dimension (it should be " + std::to_string(nu_) + ")");
  }

  ActionDataQuadrupedNonLinearTpl<Scalar>* d = static_cast<ActionDataQuadrupedNonLinearTpl<Scalar>*>(data.get());  

  // Cost derivatives : Lx
  d->Lx = (state_weights_.array()* d->r.template head<12>().array()).matrix() ;

  // Hessian : Lxx
  d->Lxx.block(2,2,3,3).setZero() ; 
  d->Lxx.diagonal() = (state_weights_.array() * state_weights_.array()).matrix() ;  

  // Shoulder height derivative cost
  for (int j=0 ; j<4 ; j=j+1){
    if (sh_ub_max_[j] > Scalar(0.) ){
      d->Lx.block(2,0,1,1) += shoulder_height_weight*sh_ub_max_.block(j,0,1,1) ; 
      d->Lx.block(3,0,1,1) += shoulder_height_weight*pshoulder_0(1,j)*sh_ub_max_.block(j,0,1,1) ; 
      d->Lx.block(4,0,1,1) += - shoulder_height_weight*pshoulder_0(0,j)*sh_ub_max_.block(j,0,1,1) ; 

      d->Lxx.block(2,2,1,1) += shoulder_height_weight*Eigen::Matrix<Scalar, 1, 1>::Identity() ; 
      d->Lxx.block(3,3,1,1) += shoulder_height_weight*pshoulder_0(1,j)*pshoulder_0.block(1,j,1,1) ; 
      d->Lxx.block(4,4,1,1) += shoulder_height_weight*pshoulder_0(0,j)*pshoulder_0.block(0,j,1,1) ; 

      d->Lxx.block(3,4,1,1) += -shoulder_height_weight*pshoulder_0(1,j)*pshoulder_0.block(0,j,1,1) ; 
      d->Lxx.block(4,3,1,1) += -shoulder_height_weight*pshoulder_0(1,j)*pshoulder_0.block(0,j,1,1) ; 
      d->Lxx.block(2,3,1,1) += shoulder_height_weight*pshoulder_0.block(1,j,1,1) ; 
      d->Lxx.block(3,2,1,1) += shoulder_height_weight*pshoulder_0.block(1,j,1,1) ; 
      d->Lxx.block(2,4,1,1) += -shoulder_height_weight*pshoulder_0.block(0,j,1,1) ; 
      d->Lxx.block(4,2,1,1) += -shoulder_height_weight*pshoulder_0.block(0,j,1,1) ; 
    }
  }  
  
  

  // Cost derivative : Lu
  for (int i=0; i<4; i=i+1){
    // r << friction_weight_*rub_max_.segment(5*i,5) ; 
    r[1] = friction_weight_*rub_max_[5*i] ;
    r[2] = friction_weight_*rub_max_[5*i + 1] ;
    r[3] = friction_weight_*rub_max_[5*i + 2] ;
    r[4] = friction_weight_*rub_max_[5*i + 3] ;
    r[5] = friction_weight_*rub_max_[5*i + 4] ;
    d->Lu.block(i*3,0,3,1) << r[1] - r[2] , r[3] - r[4] , -mu*(r[1] + r[2] + r[3] + r[4] ) - r[5] ;
  }  
  d->Lu = d->Lu + (force_weights_.array()*d->r.template tail<12>().array()).matrix() ; 
  
  
  
  // Hessian : Luu
  // Matrix friction cone hessian (20x12)
  Arr.diagonal() =  ((Fa_x_u - ub).array() >= 0.).matrix().template cast<Scalar>() ; 
  for (int i=0; i<4; i=i+1){
    // r = friction_weight_*Arr.diagonal().segment(5*i,5) ; 
    r[1] = friction_weight_*Arr.diagonal()[5*i] ;
    r[2] = friction_weight_*Arr.diagonal()[5*i+1] ;
    r[3] = friction_weight_*Arr.diagonal()[5*i+2] ;
    r[4] = friction_weight_*Arr.diagonal()[5*i+3] ;
    r[5] = friction_weight_*Arr.diagonal()[5*i+4] ;
    d->Luu.block(3*i,3*i,3,3) << r[1] + r[2] , 0.0 , mu*(r[2] - r[1] ),
                                  0.0,  r[3] + r[4] , mu*(r[4] - r[3] ), 
                                mu*(r[2] - r[1] ) , mu*(r[4] - r[3]) , mu*mu*(r[1] + r[2] + r[3] + r[4]) + r[5]  ; 
  }
  d->Luu.diagonal() = d->Luu.diagonal() + (force_weights_.array() * force_weights_.array()).matrix() ;

  // Dynamic derivatives
  d->Fx << A;

  for (int i=0; i<4; i=i+1){
    if (gait(i,0) != 0) {
      forces_3d = u.block(3*i,0,3,1) ; 
      d->Fx.block(9,0,3,1) += - dt_ * R *( base_vector_x.cross(forces_3d) ) ;
      d->Fx.block(9,1,3,1) += - dt_ * R *( base_vector_y.cross(forces_3d) ) ;
      d->Fx.block(9,2,3,1) += - dt_ * R *( base_vector_z.cross(forces_3d) ) ;
    }
  }
  d->Fu << B;
}



template <typename Scalar>
boost::shared_ptr<crocoddyl::ActionDataAbstractTpl<Scalar> > ActionModelQuadrupedNonLinearTpl<Scalar>::createData() {
  return boost::make_shared<ActionDataQuadrupedNonLinearTpl<Scalar> >(this);
}

////////////////////////////////
// get & set parameters ////////
////////////////////////////////

template <typename Scalar>
const typename Eigen::Matrix<Scalar, 12, 1>& ActionModelQuadrupedNonLinearTpl<Scalar>::get_force_weights() const {
  return force_weights_;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_force_weights(const typename MathBase::VectorXs& weights) {
  if (static_cast<std::size_t>(weights.size()) != state_->get_nx()) {
    throw_pretty("Invalid argument: "
                 << "Weights vector has wrong dimension (it should be " + std::to_string(state_->get_nx()) + ")");
  }  
  force_weights_ = weights;
}


template <typename Scalar>
const typename Eigen::Matrix<Scalar, 12, 1>& ActionModelQuadrupedNonLinearTpl<Scalar>::get_state_weights() const {
  return state_weights_;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_state_weights(const typename MathBase::VectorXs& weights) {
  if (static_cast<std::size_t>(weights.size()) != state_->get_nx()) {
    throw_pretty("Invalid argument: "
                 << "Weights vector has wrong dimension (it should be " + std::to_string(state_->get_nx()) + ")");
  }
  state_weights_ = weights;
}


template <typename Scalar>
const Scalar& ActionModelQuadrupedNonLinearTpl<Scalar>::get_friction_weight() const {
  return friction_weight_;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_friction_weight(const Scalar& weight) {
  friction_weight_ = weight;
}


template <typename Scalar>
const Scalar& ActionModelQuadrupedNonLinearTpl<Scalar>::get_mu() const {
  return mu;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_mu(const Scalar& mu_coeff) {
  mu = mu_coeff; 
}

template <typename Scalar>
const Scalar& ActionModelQuadrupedNonLinearTpl<Scalar>::get_mass() const {
  return mass;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_mass(const Scalar& m) {
  // The model need to be updated after this changed
  mass = m; 
}

template <typename Scalar>
const Scalar& ActionModelQuadrupedNonLinearTpl<Scalar>::get_dt() const {
  return dt_;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_dt(const Scalar& dt) {
  // The model need to be updated after this changed
  dt_ = dt; 
  g[8] = Scalar(-9.81)*dt_ ;
  A.topRightCorner(6,6) << Eigen::Matrix<Scalar, 6, 6>::Identity()*dt_ ; 
}

template <typename Scalar>
const typename Eigen::Matrix<Scalar, 3, 3>& ActionModelQuadrupedNonLinearTpl<Scalar>::get_gI() const {
  return gI;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_gI(const typename MathBase::Matrix3s& inertia_matrix) {
  // The model need to be updated after this changed
  if (static_cast<std::size_t>(inertia_matrix.size()) != 9) {
    throw_pretty("Invalid argument: "
                 << "gI has wrong dimension : 3x3");
  }
  gI = inertia_matrix; 
}

template <typename Scalar>
const Scalar& ActionModelQuadrupedNonLinearTpl<Scalar>::get_min_fz_contact() const {
  // The model need to be updated after this changed
  return min_fz_in_contact;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_min_fz_contact(const Scalar& min_fz) {
  // The model need to be updated after this changed
  min_fz_in_contact = min_fz; 
}

template <typename Scalar>
const Scalar& ActionModelQuadrupedNonLinearTpl<Scalar>::get_shoulder_hlim() const {
  return shoulder_height_limit;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_shoulder_hlim(const Scalar& hlim) {
  // The model need to be updated after this changed
  shoulder_height_limit = hlim; 
}

template <typename Scalar>
const Scalar& ActionModelQuadrupedNonLinearTpl<Scalar>::get_shoulder_weight() const {
  return shoulder_height_weight;
}
template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::set_shoulder_weight(const Scalar& weight) {
  // The model need to be updated after this changed
  shoulder_height_weight = weight; 
}


///////////////////////////
//// get A & B matrix /////
///////////////////////////
template <typename Scalar>
const typename Eigen::Matrix<Scalar, 12, 12>& ActionModelQuadrupedNonLinearTpl<Scalar>::get_A() const {
  return A;
}
template <typename Scalar>
const typename Eigen::Matrix<Scalar, 12, 12>& ActionModelQuadrupedNonLinearTpl<Scalar>::get_B() const {
  return B;
}


////////////////////////
// Update current model 
////////////////////////

template <typename Scalar>
void ActionModelQuadrupedNonLinearTpl<Scalar>::update_model(const Eigen::Ref<const typename MathBase::MatrixXs>& l_feet  ,
                    const Eigen::Ref<const typename MathBase::MatrixXs>& xref,
                    const Eigen::Ref<const typename MathBase::MatrixXs>& S ) {
  if (static_cast<std::size_t>(l_feet.size()) != 12) {
    throw_pretty("Invalid argument: "
                 << "l_feet matrix has wrong dimension (it should be : 3x4)");
  }
  if (static_cast<std::size_t>(xref.size()) != state_->get_nx()) {
    throw_pretty("Invalid argument: "
                 << "Weights vector has wrong dimension (it should be " + std::to_string(state_->get_nx()) + ")");
  }
  if (static_cast<std::size_t>(S.size()) != 4) {
    throw_pretty("Invalid argument: "
                 << "S vector has wrong dimension (it should be 4x1)");
  }

  xref_ = xref ; 
  gait = S ;

  R_tmp << cos(xref(5,0)),-sin(xref(5,0)),0,
      sin(xref(5,0)),cos(xref(5,0)),0,
      0,0,1.0 ; 
  
  R = (R_tmp*gI).inverse() ; // I_inv  
  lever_arms.block(0,0,2,4) = l_feet.block(0,0,2,4) ; 

  for (int i=0; i<4; i=i+1){
    if (S(i,0) != 0) {
      // set limit for normal force, (foot in contact with the ground)
      ub[5*i+4] = - min_fz_in_contact ; 

      // B update
      B.block(6 , 3*i  , 3,3).diagonal() << dt_ / mass  ,  dt_ / mass  ,  dt_ / mass  ; 
      
      //  Assuption 1 : levers arms not depends on the state, but on the predicted position (xfref)
      //  --> B will be updated with the update_B method for each calc function 

      // lever_tmp = lever_arms.block(0,i,3,1) - xref.block(0,0,3,1) ;
      // R_tmp << 0.0, -lever_tmp[2], lever_tmp[1],
      // lever_tmp[2], 0.0, -lever_tmp[0], -lever_tmp[1], lever_tmp[0], 0.0 ; 
      // B.block(9 , 3*i  , 3,3) << dt_ * R* R_tmp; 
    }
    else{
      // set limit for normal force at 0.0 
      ub[5*i+4] = 0.0 ; 
      B.block(6 , 3*i  , 3,3).setZero();
      B.block(9 , 3*i  , 3,3).setZero() ; 
    };
  } ; 
}
}

#endif
