#include "integrators.hpp"

namespace RKWKB{
    
    class Solution
    {
        private:
    
    
        public:
        // default constructor
        Solution();
        // constructor overloads
        Solution(de_system de_sys, Vector ic, double t_ini, event F_end, double r_tol=1e-4, double a_tol=0.0, double stepsize=1.0);
    
        // class data 
        double t_i, rtol, h; // current stepsize
        double t; // current time
        Vector y; // current solution vector
        Vector error; // current error on solution vector
        Vector atol;
        de_system de_sys;
        RKFsolver rkfsolver;
        WKBsolver wkbsolver;
        event f_end; // function s.t. solution terminates at f(y,t)=0
    
        // class functions
        void evolve();
        Step step(Method * method, Vector Y, double H); 
        void write();
        void update_h(Vector err, Vector scale, bool wkb, bool success);

    };
    
    Solution::Solution(){
        // default constructor for a solution object (does nothing)
    };
    
    Solution::Solution(de_system system, Vector ic, double t_ini, event F_end, double r_tol, double a_tol, double stepsize){
        // constructor for solution of a system of differential equations
        
        y = ic;
        t_i = t_ini;
        rtol = r_tol;
        atol = a_tol*Vector::Ones(y.size());
        h = stepsize;
        t = t_i;
        f_end = F_end;
        error = Vector::Zero(y.size());

        de_sys = system;
        rkfsolver = RKFsolver(de_sys.F);
        wkbsolver = WKBsolver(de_sys.F);
    };
                
    void Solution::evolve(){
        // function to compute full numerical solution of the de_system.
    
        bool sign = (f_end(y, t_i).real() <= 0.0);
        bool next_sign = sign;
        Vector y_next = y;
        Vector error_next = error;
        Vector scale = rtol*y + atol;
        double t_next = t;
        Step step_rkf, step_wkb;
        double end_error;
        bool wkb=false;
                                
        while(true){
            // keep updating stepsize until step is successful
            while(true){
                step_rkf = step(&rkfsolver, y, h);
                step_wkb = step(&rkfsolver, y, h); // for testing, only take RKF steps.
                wkb = step_rkf.error.norm() > step_wkb.error.norm();
                if(wkb){
                    y_next = step_rkf.y;
                    error_next = step_rkf.error;
                }
                else{
                    y_next = step_wkb.y;
                    error_next = step_wkb.error;
                }
                scale = rtol*y_next + atol; 
                if(error_next.norm() <= scale.norm()){
                    t_next += h;   
                    break;
                }
                else{
                    update_h(error_next, scale, wkb, false);
                };
            };

            // check if f_end has changed sign
            next_sign = (f_end(y_next, t_next).real() <= 0.0);
            end_error = abs(f_end(y_next, t_next));
            if(next_sign != sign){
                h = (t_next - t)/2.0;
                t_next = t;
            }
            else{
                y = y_next;
                t = t_next;
                error = error_next;
                // update stepsize
                update_h(error_next, scale, wkb, true);
                std::cout << "time: " << t << ", solution: " << y << std::endl;
                if(abs(end_error) < 1e-4)
                    break;
            };
        };
    };

    Step Solution::step(Method * method, Vector Y, double H){
        // function to take a single step with a given method in the numerical solution of the
        // de_system, from y with stepsize h.
    
        Step s = method->step(de_sys.F, Y, H);
        return s;
    };
    
    void Solution::update_h(Vector err, Vector scale, bool wkb, bool success){
        // updates stepsize.

        if(success){
            if(wkb)
                h*=1;
            else
                h*=pow(err.norm()/scale.norm(), -1/5.0);
        }
        else{
            if(wkb)
                h*=1;
            else
                h*=pow(err.norm()/scale.norm(), -1/4.0);
        };
    };

    void Solution::write(){
        // write current step to file
    };

}






