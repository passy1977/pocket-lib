mod pods;
mod traits;
mod session;
mod config;

mod global {
    pub const DATA_FOLDER : &str = ".pocket";
}


// pub mod pocket {
//     use crate::session::Session;
//     use std::io::Result;

    


//     pub fn init() -> Result<()> {

//         if let Ok(_) = Session::new("cos".to_owned()) {
//             //println!("Name: {}, Age: {}", name, age);
//             Result::Ok(())
//         } else {
//             Result::Ok(())
//         }



//     }
// }








// #[cfg(test)]
// mod tests {
//     use super::pocket::init;

//     const REGISTRATION : &str = r#"
//     {
//         "uuid": "de10b648-9e9c-4b63-96df-2aa662fe20ba",
//         "user_uuid": "1281d20e-4e6f-43d8-9aa5-099039f36206",
//         "host": "127.0.0.1",
//         "host_pub_key": "fookey",
//     }
//     "#;

//     #[test]
//     fn it_works() {
//         //init(REGISTRATION.to_string());
//     }
// }
