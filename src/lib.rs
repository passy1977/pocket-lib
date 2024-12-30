mod pods;

pub mod pocket {
    use crate::pods::{device::Status as DStatus, fn_pods, user::Status as UStatus};

    pub fn init() {
        fn_pods();

        let s = DStatus::Active;
        let s1 = UStatus::Active;

    }
}






#[cfg(test)]
mod tests {
    use super::pocket::init;

    #[test]
    fn it_works() {
        init();
    }
}
